#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>

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
//Config
TAXA_ATUALIZ 100
TAXA_TELA 100
DEBOUNCE_MS 300
CALIB_MS
PESO_ESTIMADO

TELA_RS
TELA_EN
TELA_D4
TELA_D5
TELA_D6
TELA_D7

SD_CS

BOTAO_MEIO 


//Não mexer
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
    
      switch (estado){
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

  //inicializar o HX711

  //inicializar o Cartão SD

  if(HX711 && CARTAOSD)
  estado = 'c';
  
}

void calibra(){
  
}

void espera(){
  
}

void trabalhando(){
  
}

int medir(){

  return peso;
}

}

