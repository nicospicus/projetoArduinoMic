void setup() {
  
  /*  Definição dos registradores
   *  
   *  TIMER A
   *  Definido para gerar interrupção a cada 1 segundo (configurar contador e pre-scaler).
   *  
   *  CONFIGURAR INTERRUPÇÕES EXTERNAS, SERIAL, ETC.
   */

}

ISR(//TIMER1_COMPA_vect){
  // Inicializa OCR1A com (tempo_overflow - tempo_inicial) = (ciclos necessarios para 1s)
  /* 
   *  Nessa interrupção é legal só alterar um booleano, pra executar poucas ações.
   */
   
  // booleano que indica a passagem de 1 segundo = true.
}

ISR(//interrupção de leitura de serial){
  // Tem que ver como funciona, para que serve a interrupção.

  // O serial servirá para ler alterações do tempo do alarme e hora atual.
  // PESQUISAR SOBRE ISSO.
}

ISR(//interrupção externa 1 e 2){
  // Ativa booleanos que indicam que os botões foram pressionados.

  // Só ativa o booleano quando o botão for solto (FALLING EDGE) para evitar que o sistema siga infinitamente por alterações.
}

void loop() {
  
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
