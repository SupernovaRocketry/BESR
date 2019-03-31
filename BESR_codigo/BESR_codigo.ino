

/*
  _____                                                        ______               _           _                
/  ___|                                                       | ___ \             | |         | |               
\ `--.  _   _  _ __    ___  _ __  _ __    ___  __   __  __ _  | |_/ /  ___    ___ | | __  ___ | |_  _ __  _   _ 
 `--. \| | | || '_ \  / _ \| '__|| '_ \  / _ \ \ \ / / / _` | |    /  / _ \  / __|| |/ / / _ \| __|| '__|| | | |
/\__/ /| |_| || |_) ||  __/| |   | | | || (_) | \ V / | (_| | | |\ \ | (_) || (__ |   < |  __/| |_ | |   | |_| |
\____/  \__,_|| .__/  \___||_|   |_| |_| \___/   \_/   \__,_| \_| \_| \___/  \___||_|\_\ \___| \__||_|    \__, |
              | |                                                                                          __/ |
              |_|                                                                                         |___/ 

BESR: Banco Estático Supernova Rocketry

Toda a parte editável pelo usuário está em //config
Conexões:

 */

//Bibliotecas
#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>
#include "HX711.h"

//Configurações
TAXA_ATUALIZ 100
TAXA_TELA 100
DEBOUNCE_MS 300
CALIB_MS
PESO_ESTIMADO
DELTA_RUIDO

TELA_RS
TELA_EN
TELA_D4
TELA_D5
TELA_D6
TELA_D7

SD_CS

BOTAO_MEIO 


//Variáveis do código, não mexer
char  menu = 'e';
char  submenu;

int   peso = 0;
int   maiorPeso = 0;

int   millisAtual, millisAnterior;
int   millisTela;
int   millisBotao;
int   millisExpira;

File arquivoLog;
char nomeBase[] = "BESR";
char nomeConcat[12];


void setup() {
  LiquidCrystal tela(TELA_RS, TELA_EN, TELA_D4, TELA_D5, TELA_D6, TELA_D7);
  tela.begin(16,2); 
}

void loop() {
  
  millisAtual = millis();

  //Loop principal do banco estático
  
  if(millisAtual - millisAnterior > TAXA_ATUALIZ){
    
      switch (menu){
        case 'i':
        inicializa();
        break;
    
        case 'c':
        calibra();
        break;
    
        case 'e':
        espera();
        break
    
        case 't':
        trabalhando();
        break;
        
      }

      millisAnterior = millisAtual;
  }


  //Atualizando a tela caso passado o tempo definido
  if(millisAtual - millisTela > TAXA_TELA){
  tela.clear();
  millisTela = millisAtual;
  }
  
  

  

}



void inicializa(){

  switch(submenu){

    case 'a':
    lcd.setCursor(0, 0);
    lcd.print("  Iniciando...  ");
    
    break;

    case 'b':
    lcd.setCursor(0, 0);
    lcd.print("   Erro no SD   ");
    
    break;

    case 'c':
    lcd.setCursor(0, 0);
    lcd.print("   Erro no HX   ");
    
    break;

  }

  //inicializar o Cartão SD
  if(!SD.begin(PINO_SD_CS))
  submenu = 'b';
  else{

    int n = 1;
    bool parar = false;


    while (!parar)
    {
      sprintf(nomeConcat, "BESR%d.txt", n);
      if (SD.exists(nomeConcat))
          n++;
          else
            parar = true;
      }

  arquivoLog = SD.open(nomeConcat, FILE_WRITE);
  }
  
  //inicializar o HX711
  if(!tentariniciarHX)
  submenu = 'c';

  if(HX711 && CARTAOSD){
  menu = 'c';
  submenu = 'a';
  }
}

void calibra(){

  switch(submenu){

    case 'a':
    lcd.setCursor(0, 0);
    lcd.print("Calibrar o banco");
    lcd.setCursor(1, 0);
    lcd.print("   [Calibrar]   ");

    if(debounceBotao)
    submenu = 'b';
    
    break;

    case 'b':
    lcd.setCursor(0, 0);
    lcd.print("Posicione o peso");
    lcd.setCursor(1, 0);
    lcd.print("      [OK]      ");

    if(debounceBotao){
    submenu = 'c';
    millisExpira = millisAtual;
    }
    break;

    case 'c':
    lcd.setCursor(0, 0);
    lcd.print("Valor atual:");
    lcd.setCursor(1, 0);
    lcd.print(medir());
    
    if(millisAtual - millisExpira > CALIB_MS){

      if(maiorPeso > PESO_ESTIMADO)
      submenu = 'd';
      else
      submenu = 'e';
        
    }
    
    break;

    case 'd':
    lcd.setCursor(0, 0);
    lcd.print("Calib. concluida");
    lcd.setCursor(1, 0);
    lcd.print("      [OK]      ");

    if(debounceBotao){
    submenu = 'a';
    menu = 'e';
    }
    
    break;

    case 'e':
    lcd.setCursor(0, 0);
    lcd.print("Erro! Verif cabo");
    lcd.setCursor(1, 0);
    lcd.print("    [Voltar]    ");

    if(debounceBotao)
    submenu = 'a';
    
    break;

  }

  }
  

void espera(){

  lcd.setCursor(0, 0);
  lcd.print("Pronto p/ medir");
  lcd.setCursor(1, 0);
  lcd.print("   [Iniciar]   ");

  if(debounceBotao){
    menu = 't';
    submenu = 'a';
  }
  
}

void trabalhando(){

  switch(submenu){

    case 'a':
    medir();
    escrever();
    
    lcd.setCursor(0, 0);
    lcd.print("A:      M:      ");
    
    lcd.setCursor(0, 3);
    lcd.print(peso/100);

    lcd.setCursor(0, 12);
    lcd.print(maiorPeso/100);
    
    
    lcd.setCursor(1, 0);
    lcd.print("   [Terminar]   ");

    if(debounceBotao)
    submenu = 'b';
    
    break;

    case 'b':
    lcd.setCursor(0, 0);
    lcd.print("Salvo:");
    lcd.setCursor(0, 7);
    lcd.print(nomeConcat);
    lcd.setCursor(1, 0);
    lcd.print("      [OK]      ");

    if(debounceBotao)
    menu = 'e';
    
    break;

  }
  
}

void medir(){

    peso = lerHX;

    if(peso > maiorPeso)
    maiorPeso = peso;

}

bool debounceBotao{

  if(DigitalRead(PINO_ENTER))
    botaoApertado = 1;

  if((botaoApertado == 1) && (millisAtual - millisBotao > DEBOUNCE_MS)){
    return 1;
    botaoApertado = 0;

  }
  else
  return 0;
}

void escrever(){

  arquivoLog = SD.open(nomeConcat, FILE_WRITE);
  
  stringDados += millisGravacao;
  stringDados += ",";
  stringDados += peso;
  stringDados += ",";
  stringDados += maiorPeso;
  arquivoLog.println(stringDados);
  
  arquivoLog.close();
}


