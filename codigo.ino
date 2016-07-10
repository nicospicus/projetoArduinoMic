// DEFINIÇÃO DE VARIÁVEIS DO PROJETO.
unsigned int segundos = 0;
unsigned int minutos = 0;
unsigned int horas = 0;

unsigned int alarme_minutos = 0;
unsigned int alarme_horas = 0;
volatile boolean alarme_ativado = false;
volatile boolean alarme_tocando = false;

volatile boolean leu_string = false;
unsigned char buffer_leitura[50];
unsigned int buffer_index = 0;

unsigned int bateria = 100;

const char mensagem_erro[] = "Comando invalido!";

void setup() 
{
  //=========== Configuração do USART0 em modo Assíncrono para Comunicação Serial. ===========
  UBRR0 = 103; // Para definir o BAUD Rate como 9600, calcula-se UBR0 = [freq/(16*BAUD)] - 1
  UCSR0A = 0b00000000;
  // Habilita a interrupção de fim de leitura de dados e habilita a recepção de dados.
  UCSR0B = 0b00000000 | ( 1 << RXCIE0 ) | ( 1 << RXEN0 ) | ( 1 << TXEN0 );
  // Define: operação em modo assíncrono; sem paridade; 1 stop bit; 8 bits; 1 stop bit
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
  EIMSK = (1 << INT0);
  EICRA = (1 << ISC01) | (0 << ISC00);
  //Acionamento configurado em 'FALLING EDGE' para evitar que o comando seja enviado indeterminadamente.
  //(Caso a pessoa segure o botão)
  
  //Segundo botão:
  EIMSK = (1 << INT1);
  EICRA = (1 << ISC11) | (0 << ISC10);
  //Da mesma forma, acionamento em 'FALLING EDGE'
}

// Interrupção executada toda vez que TCTN1 = OCR1A. Ou seja, conforme as configurações, quando ele atinge 1s.
ISR(TIMER1_COMPA_vect)    //Interrupção do modo CTC do Timer1
{
  segundos = segundos + 1;
}

// Interrupção dispara quando toda a leitura de 8 bits foi terminada.
ISR(USART_RX_vect)
{
  buffer_leitura[buffer_index++] = UDR0;
  if(buffer_leitura[buffer_index] == 0)
  {
    leu_string = true;
  }
}

//Imprime no serial.
void imprimir(char[] texto)
{
    int i = 0;
    while(texto[i] != '\0')
    {
	while( !(UCSR0A & ( 1 << TXC0 )) ); //Espera estar livre pra escrever
        UDR0 = buffer_leitura[i++];
    }
}


//Interrupcao para o botao 1, que ativa/desativa alarme.
ISR(INT0_vect)
{
  alarme_ativado = !alarme_ativado;
  //Falta ligar alguma coisa que mostre o estado do alarme.
}
 
//Interrupcao para o botao 2, que faz com que o alarme pare de tocar.
ISR(INT1_vect)
{
  alarme_tocando = false;
}
 

void loop() {
  // Verificação de troca entre segundos/minutos/horas.
  if (segundos >= 60)
  {
    segundos = 0;
    minutos = minutos + 1;
    bateria--; //A cada minuto, perde 1% de bateria.
    //Aqui, vamos usar o valor lido do potenciometro para adicionar um valor
    //entre 0 e 3 (ou outros valores) à bateria.

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
  
  /*
   * Faz as comparações de sempre para passagem do tempo.
   * Se segundo_unidade > 9 então segundo_dezena++.
   * Se segundo dezena > 5 então minuto_unidade++
   * e assim por diante.
   * Depois disso, fazer a logica de transmissao para o display.
   */

  /*
   * Comparação com o tempo do alarme, se der igual toca!
   */
   if(alarme_ativado)
   {
	if(horas == alarme_horas && minutos == alarme_minutos)
	{
	     //Toca alarme
	     //Como faremos para tocar o alarme?
	     //Será que conseguimos gerar algum som pelo computador, ou será só visual?
	     alarme_tocando = true;
	}
   }
  
   if(alarme_tocando)
   {
	   //Do stuff
	   //Seria interessante não deixar tocando do mesmo jeito sempre.
	   //Da para fazer com que a frequência do toque aumente com o tempo, ou que o alarme pare depois de uma hora.
   }

   // Se lida uma string por completo, uma linha inteira estará presente no buffer_leitura.
  if(leu_string)
   {
    // Por ora, coloquei Print dos caracteres inseridos no buffer.
    i = 0;
     while(buffer_leitura[i] != '\0')
     {
	while( !(UCSR0A & ( 1 << TXC0 )) );
        UDR0 = buffer_leitura[i++];
     }
	// Aqui termina a parte temporária.
	
 
	//Verifica se o comando faz sentido
	if((buffer_leitura[1]*10 + buffer_leitura[2] >= 0 || buffer_leitura[1]*10 + buffer_leitura[2] < 24)
		&& (buffer_leitura[4]*10 + buffer_leitura[5] >= 0 || buffer_leitura[4]*10 + buffer_leitura[5] < 60))
	{
		//Configura hora
		if(buffer_leitura[0] == 'H' || buffer_leitura[0] == "h")
		{
			horas = buffer_leitura[1]*10 + buffer_leitura[2];
			minutos = buffer_leitura[4]*10 + buffer_leitura[5];
		}	 
		//Configura alarme
		else if(buffer_leitura[0] == 'A' || buffer_leitura[0] == 'a')
		{
			alarme_horas = buffer_leitura[1]*10 + buffer_leitura[2];
			alarme_minutos = buffer_leitura[4]*10 + buffer_leitura[5];
		}	
		else //Se o comando for invalido
			imprimit(mensagem_erro);
	}
	else //Se o comando for invalido.
		imprimir(mensagem_erro);

     // Esvazia o que está presente no buffer_leitura e retorna as variáveis booleanas aos valores originais.
     leu_string = false;
     buffer_index = 0;
     buffer_leitura[0] = '\0';
   }
   

  /*
   * PENSAR SOBRE ALGO PARA CONVERSÃO A/D!
     Ficaria bem legal utilizar conversão A/D.
     Tenho uma sugestão!
     Por quê não fazemos um esquema de bateria no relógio? 
     Poderíamos utilizar um potenciômetro para indicar a quantidade de energia sendo recebida pelo relógio.
     No caso, seria um relógio com carregamento por energia solar.
     Daria para ir regulando a incidência de sol pelo potenciômetro ou então fazer uma programação para
     gerar valores aleatório a cada hora, algo assim.
     
     O que acham? Só uma ideia mesmo.
   */  
}
