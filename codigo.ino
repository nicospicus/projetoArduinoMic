
// DEFINIÇÃO DE VARIÁVEIS DO PROJETO.
int segundos = 0;
int minutos = 0;
int horas = 0;

void setup() {

  Serial.begin(9600);

  // Configuração do Timer 1 (de 16 bits) para contagem dos segundos.

  // Inicialmente, coloca valor zero nos registradores do Timer 1.
  TCCR1A = 0;
  TCCR1B = 0;
  TIMSK1 = 0;
  
  // Configura o Prescaler para frequência de (16MHz/1024) = 15625Hz.
  TCCR1B |= ( 1 << CS12 ) | ( 0 << CS11 ) | ( 1 << CS10 );

  // Zera o valor inicial do Timer.
  TCNT1 = 0;

  // O valor máximo do counter será o número de ticks do pre-scaler para gerar 1s.
  // Com frequência de 15625Hz (16Mhz/1024), serão necessários 15625 ticks para obter 1s.
  OCR1A = 15625;

  // Define o modo do Timer 1 como CTC (Clear Timer on Compare) com OCR1A.
  TCCR1A |= ( 0 << WGM11 ) | ( 0 << WGM10 );
  TCCR1B |= ( 0 << WGM13 ) | ( 1 << WGM12 );

  // Habilita a interrupção ativada quando o valor do timer se torna igual ao valor do OCR1A.
  TIMSK1 |= ( 1 << OCIE1A );

  
  /*  Definição dos registradores
   *  
   *  TIMER A
   *  Definido para gerar interrupção a cada 1 segundo (configurar contador e pre-scaler).
   *  
   *  CONFIGURAR INTERRUPÇÕES EXTERNAS, SERIAL, ETC.
   */

}

ISR(TIMER1_COMPA_vect){
  // Interrupção executada toda vez que TCTN1 = OCR1A. Ou seja, conforme as configurações, quando ele atinge 1s.
  segundos = segundos + 1;

  // Print temporário a cada alteração de segundos, para verificar a validade da implementação.
  Serial.println( String(horas) + ':' + String(minutos) + ':' + String(segundos) );
}

//ISR(//interrupção de leitura de serial){
  // Tem que ver como funciona, para que serve a interrupção.

  // O serial servirá para ler alterações do tempo do alarme e hora atual.
  // PESQUISAR SOBRE ISSO.
//}

//ISR(interrupção externa 1 e 2){
  // Ativa booleanos que indicam que os botões foram pressionados.

  // Só ativa o booleano quando o botão for solto (FALLING EDGE) para evitar que o sistema siga infinitamente por alterações.
//}

void loop() {
  // Verificação de troca entre segundos/minutos/horas.
  if (segundos >= 60){
    segundos = 0;
    minutos = minutos + 1;

    if (minutos >= 60){
      minutos = 0;
      horas = horas + 1;

      if (horas >= 24){
        horas = 0;
      }
    }
  }
  /*
   *  -- quando a interrupção for executada, soma 1 no segundo
   * if (booleano de passagem de 1 segundo = true){
   *  incrementa 1 em segundo a variável de unidade dos segundos.
   *  booleano de passagem de segundo = false
   *  
   *  Possivelmente reseta o timer.
   * }
   */

  /*
   * Faz as comparações de sempre para passagem do tempo.
   * Se segundo_unidade > 9 então segundo_dezena++.
   * Se segundo dezena > 5 então minuto_unidade++
   * e assim por diante.
   */

  /*
   * Comparação com o tempo do alarme, se der igual toca!
   */

  /*
   * Verificar se há dados inseridos no Serial para leitura.
   * 
   * PESQUISAR SOBRE ISSO!
   */

  /*
   * Ver se o botão foi pressionado para ativar/desativar alarme ou parar de tocar.
   * Dois botões: alarme e soneca.
   */

  /*
   * PENSAR SOBRE ALGO PARA CONVERSÃO A/D!
   */
}
