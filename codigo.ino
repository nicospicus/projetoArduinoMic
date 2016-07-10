// DEFINIÇÃO DE VARIÁVEIS DO PROJETO.
unsigned int segundos = 0;
unsigned int minutos = 0;
unsigned int horas = 0;

volatile boolean leu_string = false;
unsigned char buffer_leitura[50];
unsigned int buffer_index = 0;

void setup() {

  // Configuração do USART0 em modo Assíncrono para Comunicação Serial.
  // Para definir o BAUD Rate como 9600, calcula-se [freq/(16*BAUD)] - 1
  UBRR0 = 103;

  UCSR0A = 0b00000000;
  // Habilita a interrupção de fim de leitura de dados e habilita a recepção de dados.
  UCSR0B = 0b00000000 | ( 1 << RXCIE0 ) | ( 1 << RXEN0 ) | ( 1 << TXEN0 );  // DESABILITAR O TXEN AO FINAL, USADO EXPERIMENTALMENTE.
  // Define: operação em modo assíncrono; sem paridade; 1 stop bit; 8 bits; 1 stop bit
  UCSR0C = 0b00000000 | ( 1 << UCSZ01 ) | ( 1 << UCSZ00 );

  // Configuração do Timer 1 (de 16 bits) para contagem dos segundos.

  // Configura o Prescaler para frequência de (16MHz/1024) = 15625Hz.
  TCCR1B = 0b00000000 | ( 1 << CS12 ) | ( 0 << CS11 ) | ( 1 << CS10 );

  // Zera o valor inicial do Timer.
  TCNT1 = 0;

  // O valor máximo do counter será o número de ticks do pre-scaler para gerar 1s.
  // Com frequência de 15625Hz (16Mhz/1024), serão necessários 15624 ticks para obter 1s.
  OCR1A = 15624;   // No modo CTC, sempre que o tempo desejado é atingido, ele se reseta.
                   //Como o reset leva um ciclo de clock para ser concluído, temos 15625-1=15624

  // Define o modo do Timer 1 como CTC (Clear Timer on Compare) com OCR1A.
  TCCR1A = 0b00000000 | ( 0 << WGM11 ) | ( 0 << WGM10 );
  TCCR1B |= ( 0 << WGM13 ) | ( 1 << WGM12 );

  // Habilita a interrupção ativada quando o valor do timer se torna igual ao valor do OCR1A.
  TIMSK1 = 0b00000000 | ( 1 << OCIE1A );

  
  /*  Definição dos registradores
   *  
   *  TIMER A
   *  Definido para gerar interrupção a cada 1 segundo (configurar contador e pre-scaler).
   *  
   *  CONFIGURAR INTERRUPÇÕES EXTERNAS, SERIAL, ETC.
   */

}

// Interrupção executada toda vez que TCTN1 = OCR1A. Ou seja, conforme as configurações, quando ele atinge 1s.
ISR(TIMER1_COMPA_vect)
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


//ISR(interrupção externa 1 e 2){
  // Ativa booleanos que indicam que os botões foram pressionados.

  // Só ativa o booleano quando o botão for solto (FALLING EDGE) para evitar que o sistema siga infinitamente por alterações.
//}

void loop() {
  // Verificação de troca entre segundos/minutos/horas.
  if (segundos >= 60)
  {
    segundos = 0;
    minutos = minutos + 1;

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
   * Depois disso, fazer a logica de transimssao para o display.
   */

  /*
   * Comparação com o tempo do alarme, se der igual toca!
   */

   // Se lida uma string por completo, uma linha inteira estará presente no buffer_leitura.
  if(leu_string)
   {
     // DEFINIR OS NOVOS COMANDOS AO TERMINAR LEITURA AQUI. DEFINIR PROTOCOLO DO TIPO 'a11:20' ativa o alarme ou 'h20:20' muda a hora.
    
     // Por ora, coloquei Print dos caracteres inseridos no buffer.
     for( int i = 0; i < sizeof(buffer_leitura); i++)
     {
        UDR0 = buffer_leitura[i];
        while( !(UCSR0A & ( 1 << TXC0 )) );
     }
     // Aqui termina a parte temporária.

     // Esvazia o que está presente no buffer_leitura e retorna as variáveis booleanas aos valores originais.
     leu_string = false;
     buffer_index = 0;
     for(int i = 0; i < sizeof(buffer_leitura); i++)
     {
       // Limpa todo o buffer_leitura.
       buffer_leitura[i] = (char)0;
     }
   }
   

  /*
   * Ver se o botão foi pressionado para ativar/desativar alarme ou parar de tocar.
   * Dois botões: alarme e soneca.
   */

  /*
   * PENSAR SOBRE ALGO PARA CONVERSÃO A/D!
   */
}
