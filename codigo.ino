#include <LiquidCrystal.h>
// LiquidCrystal(RS, EN, D4, D5, D6, D7) -- ordem de conexão dos pinos, conforme a library.
LiquidCrystal lcd(12, 13, 11, 10, 9, 8);

// DEFINIÇÃO DE VARIÁVEIS DO PROJETO.
unsigned int segundos = 0;
unsigned int minutos = 0;
unsigned int horas = 0;

unsigned int alarme_minutos = 0;
unsigned int alarme_horas = 0;
volatile boolean alarme_ativado = true;
volatile boolean alarme_tocando = false;
volatile boolean alarme_lock = false;

volatile boolean leu_string = false;
char buffer_leitura[100];
unsigned int buffer_index = 0;

int bateria = 100;
unsigned int luz_solar = 0;

char mensagem_erro[] = "Comando invalido!\r";

volatile boolean fim_conversao = false;

void setup() 
{
  //=========== Configuração do USART0 em modo Assíncrono para Comunicação Serial. ===========
  UBRR0 = 103; // Para definir o BAUD Rate como 9600, calcula-se UBR0 = [freq/(16*BAUD)] - 1
  UCSR0A = 0b00000000;
  // Habilita a interrupção de fim de leitura de dados e habilita a recepção de dados.
  UCSR0B = 0b00000000 | ( 1 << RXCIE0 ) | ( 1 << RXEN0 ) | ( 1 << TXEN0 );
  // Define: operação em modo assíncrono; sem paridade; 1 stop bit; 8 bits.
  UCSR0C = 0b00000000 | ( 1 << UCSZ01 ) | ( 1 << UCSZ00 );
  

  //=========== Configuração do Timer 1 (de 16 bits) para contagem dos segundos. ===========
  // Configura o Prescaler para frequência de (16MHz/1024) = 15625Hz.
  TCCR1B = 0b00000000 | ( 1 << CS12 ) | ( 0 << CS11 ) | ( 1 << CS10 );
  TCNT1 = 0;  // Zera o valor inicial do Timer.
  
  // O valor máximo do counter será o número de ticks do pre-scaler para gerar 1s.
  // Com frequência de 15625Hz, serão necessários 15625 ticks para obter 1s.
  // Mas, no modo CTC, sempre que o tempo desejado é atingido, ele se reseta.
  // Como o reset leva um ciclo de clock para ser concluído, temos 15625-1=15624
  OCR1A = 15624; 
  
  // Define o modo do Timer 1 como CTC (Clear Timer on Compare) com OCR1A.
  TCCR1A = 0b00000000 | ( 0 << WGM11 ) | ( 0 << WGM10 );
  TCCR1B |= ( 0 << WGM13 ) | ( 1 << WGM12 );

  // Habilita a interrupção ativada quando o valor do timer se torna igual ao valor do OCR1A.
  TIMSK1 = 0b00000000 | ( 1 << OCIE1A );
   
   
  //=========== Configuração das interrrupções externas: =========== 
  //Primeiro botão:
  EIMSK = 0 | (1 << INT0);
  EICRA = 0 | (1 << ISC01) | (0 << ISC00);
  //Acionamento configurado em 'FALLING EDGE' para evitar que o comando seja enviado indeterminadamente.
  
  //Segundo botão:
  EIMSK |= (1 << INT1);
  EICRA |= (1 << ISC11) | (0 << ISC10);
  //Da mesma forma, acionamento em 'FALLING EDGE'

  //=========== Configuração do display LCD: =========== 
  // Inicia a exibição no display LCD, definindo um display de 16 colunas e 2 linhas.
  lcd.begin(16, 2);
  
  //==============Configuração  ADC (Conversor Analógico-Digital).=============
  ADMUX = (1<<REFS0); //channel = 0 e REFSO = 1 -> VCC como referencia de tensao.

  // Habilita o ADC e define prescaler de 128
  // 16000000/128 = 125000
  ADCSRA = (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
  ADCSRA |= (1<<ADSC); //Inicia a primeira conversao.
}

// Interrupção executada toda vez que TCTN1 = OCR1A (modo CTC). Ou seja, conforme as configurações, quando ele atinge 1s.
ISR(TIMER1_COMPA_vect)
{
  segundos = segundos + 1;
  
  bateria-=2; //A cada minuto, perde 1% de bateria.
  if(fim_conversao)
  {
    fim_conversao = false;
    luz_solar = ADC;//L + (ADCH<<8);
    ADCSRA |= (1<<ADSC);
  }
  bateria += luz_solar*5/1023.0;
  if(bateria>100) //Satura bateria
    bateria = 100;
  if(bateria<0)
    bateria = 0;
}

// Interrupção dispara quando toda a leitura de 8 bits foi terminada.
ISR(USART_RX_vect)
{
  buffer_leitura[buffer_index] = UDR0;
  if(buffer_leitura[buffer_index++] == '\r')
  {
    leu_string = true;
  }
}

//Imprime no serial.
void imprimir(char texto[])
{
  int i = 0;
  while(texto[i] != '\0')
  {
    while( !(UCSR0A & ( 1 << UDRE0 )) ); //Espera estar livre pra escrever
    UDR0 = texto[i++];
  }
}

//Interrupcao para o botao 1, que ativa/desativa alarme.
ISR(INT0_vect)
{
  alarme_ativado = !alarme_ativado;
}
 
//Interrupcao para o botao 2, que faz com que o alarme pare de tocar.
ISR(INT1_vect)
{
  alarme_tocando = false;
  alarme_lock = true; //Trava o alarme, para que nao toque mais nesse minuto.
}

//Interrupcao que sinaliza o fim da conversao AD.
ISR(ADC_vect)
{
  fim_conversao = true;
}

// Rotina que leva ao display tudo que deve ser exibido, conforme as configurações adotadas e as variáveis.
void exibe_display(boolean tem_bateria)
{
  if(tem_bateria)
  {
      // Coloca o cursor na posição inicial das horas.
      lcd.setCursor(0,0);
      
      // Coloca um zero à frente das horas, caso seja um dígito menor que dez.
      if(horas < 10)
        lcd.print( "0" + String(horas) );
      else
        lcd.print( String(horas) );
        
      lcd.print(":");
      
      // Repete o processo para os minutos e segundos.
      if(minutos < 10)
        lcd.print( "0" + String(minutos) );
      else
        lcd.print( String(minutos) );
    
      lcd.print(":");
      
      if(segundos < 10)
        lcd.print( "0" + String(segundos) );
      else
        lcd.print( String(segundos) );
    
      if(bateria==100)
        lcd.print( " " + String(bateria) + "%");
      else if(bateria<100 && bateria >= 10)
        lcd.print( " " + String(bateria) + "% ");
      else if(bateria<10)
        lcd.print( " " + String(bateria) + "%  ");
    
      // Se o alarme estiver acionado, exibe um caractere no canto da tela indicando.
      if(alarme_ativado)
      {
        lcd.setCursor(15,0);
        lcd.print("A");
      }
      else
      {
        lcd.setCursor(15,0);
        lcd.print(" ");
      }
      // Se o alarme estiver tocando (o que é definido na rotina principal), exibe a mensagem.
      if(alarme_tocando)
      {
        lcd.setCursor(0,1);
        lcd.print("ALARME TOCANDO!");
      }
      else
      {
        lcd.setCursor(0,1);
        lcd.print("               ");
      }
  }
  else //Se nao tiver bateria, limpa a tela.
  {
    lcd.setCursor(0,0);
    lcd.print("                 ");
    lcd.setCursor(0,1);
    lcd.print("                 ");
  }
  
  
}

void loop() {
  
  // Verificação de troca entre segundos/minutos/horas.
  if (segundos >= 60)
  {
    segundos = 0;
    minutos = minutos + 1;
    alarme_lock = false; //Quando passa 1 minutos, pode-se destravar o alarme.

    if (minutos >= 60)
    {
      minutos = 0;
      horas = horas + 1;

      if (horas >= 24)
      {
        horas = 0;
      }
    }
  }
  
  //O alarme_lock é para quando o alarme está tocando e o botao de parar alarme é pressionada.
  //Se nao tivesse isso, o alarme voltaria a tocar.
  if(alarme_ativado && !alarme_lock)
  {
    if(horas == alarme_horas && minutos == alarme_minutos)
    {
      // Ativa a variável alarme_tocando. Na rotina exibe_display, teremos a exibição de uma mensagem exibida.
      alarme_tocando = true;
    }
  }
  

  // Se lida uma string por completo, uma linha inteira estará presente no buffer_leitura.
  if(leu_string)
  {
    UCSR0B |= ( 0 << RXCIE0 ); //desliga interrupcao, pra garantir que nada seja escrito enquanto esta sendo lido.
    
    int hora1 = buffer_leitura[1] - '0';
    int hora2 = buffer_leitura[2] - '0'; //Converte string para int.
    int min1 = buffer_leitura[4] - '0';
    int min2 = buffer_leitura[5] - '0';
    
    // Verifica se o comando faz sentido para HH:MM:SS
    if((hora1*10 + hora2 >= 0 && hora1*10 + hora2 < 24)
      && (min1*10 + min2 >= 0 && min1*10 + min2 < 60))
    {
      // Configura hora (colocando a letra 'H' ou 'h' na frente do comando).
      if(buffer_leitura[0] == 'H' || buffer_leitura[0] == 'h')
      {
        horas = hora1*10 + hora2;
        minutos = min1*10 + min2;
      }  
      
      // Configura alarme (colocando a letra 'A' ou 'a' na frente do comando).
      else if(buffer_leitura[0] == 'A' || buffer_leitura[0] == 'a')
      {
        alarme_horas = hora1*10 + hora2;
        alarme_minutos = min1*10 + min2;
      } 
      else // Se o comando for invalido
        imprimir(mensagem_erro);
    }
    else //Se o comando for invalido.
      imprimir(mensagem_erro);

    // Esvazia o que está presente no buffer_leitura e retorna as variáveis booleanas aos valores originais.
    leu_string = false;
    buffer_index = 0;
    buffer_leitura[0] = '\r';
    UCSR0B |= ( 1 << RXCIE0 ); //Liga interrupcao apos leitura.
  }

  // Promove a exibição no display das variáveis necessárias.
  if(bateria>5) //Se tiver com bateria, mostra a hora.
    exibe_display(true);
  else  //Se acabou a bateria, nao mostra nada, mas continua rodando o clock.
    exibe_display(false);
}
