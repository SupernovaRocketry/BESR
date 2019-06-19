

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
#define TAXA_ATUALIZ 50
#define TAXA_TELA 100
#define DEBOUNCE_MS 500
#define CALIB_MS 5000
#define PESO_ESTIMADO 250.0
#define DELTA_RUIDO 1500
#define HX_DELAY_INICIO 2000
//#define DEBUG_SERIAL

const int TELA_RS = 2;
const int TELA_EN = 3;
const int TELA_D4 = 4;
const int TELA_D5 = 5;
const int TELA_D6 = 6;
const int TELA_D7 = 7;

const int PINO_SD_CS = 8;

#define PINO_BOTAO A5
#define TAMANHO_BUFFER 512

//Variáveis do código, não mexer
char  menu = 'i';
char  submenu = 'a';

float   peso = 0;
float   maiorPeso = 0;

int   millisAtual, millisAnterior;
int   millisTela;
int   millisExpira;
int   telaMudou = 0;

int estadoBotao;
int ultimoEstadoBotao;
unsigned long ultimoBotaoApertado = 0;
unsigned long botaoApertado = 0;

File arquivoLog;
char nomeBase[] = "BESR";
char nomeConcat[12];
String stringDados;

const int   HX_DOUT = A0;
const int   HX_CLK = A1;
const float FATOR_CALIB = 2.00;
HX711 celulaCarga(HX_DOUT, HX_CLK);

LiquidCrystal tela(TELA_RS, TELA_EN, TELA_D4, TELA_D5, TELA_D6, TELA_D7);

int debounceBotao();
void medir();
void escrever();
void finaliza();
void espera();

void setup() {
  //Setup do display e inicialização

  pinMode(PINO_BOTAO, INPUT);
  tela.begin(16,2);

  #ifdef DEBUG_SERIAL 
  Serial.begin(115200);
  #endif
  
}

void loop() {
  
  millisAtual = millis();

  //Loop principal do banco estático
  
  if(millisAtual - millisAnterior > TAXA_ATUALIZ){
      
      #ifdef DEBUG_SERIAL 
      Serial.print("Menu: ");
      Serial.print(menu);
      Serial.print(" Submenu: ");
      Serial.println(submenu);
      #endif
        
      switch (menu){
        case 'i':
        inicializa();
        break;
    
        case 'c':
        calibra();
        break;
    
        case 'e':
        espera();
        break;
    
        case 't':
        trabalhando();
        break;
        
      }

      millisAnterior = millisAtual;
  }


  //Atualizando a tela caso passado o tempo definido
  if((millisAtual - millisTela > TAXA_TELA) && telaMudou ){
  tela.clear();
  telaMudou = 0;
  millisTela = millisAtual;
  }
}



void inicializa(){

  switch(submenu){

    case 'a':
    tela.setCursor(0, 0);
    tela.print("  Iniciando...  ");
    
    break;

    case 'b':
    tela.setCursor(0, 0);
    tela.print("   Erro no SD   ");
    
    break;

    case 'c':
    tela.setCursor(0, 0);
    tela.print("   Erro no HX   ");
    
    break;

  }

  //inicializar o Cartão SD
  if(!SD.begin(PINO_SD_CS)){
  submenu = 'b';
  telaMudou = 1;
  }
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
  //Setup da célula de carga, sua inicialização é feita posteriormente
  
  delay(HX_DELAY_INICIO);
  celulaCarga.set_scale(FATOR_CALIB);
  float zero = celulaCarga.get_units(100);
  celulaCarga.tare(zero);

  if(celulaCarga.is_ready()){
  submenu = 'c';
  telaMudou = 1;
  }

  if(submenu == 'a'){
  menu = 'c';
  submenu = 'a';
  telaMudou = 1;
  }
}

void calibra(){

  switch(submenu){

    case 'a':
    tela.setCursor(0, 0);
    tela.print("Calibrar o banco");
    tela.setCursor(0, 1);
    tela.print("   [Calibrar]   ");

    if(debounceBotao()){
    submenu = 'b';
    telaMudou = 1;
    }
    
    break;

    case 'b':
    tela.setCursor(0, 0);
    tela.print("Posicione o peso");
    tela.setCursor(0, 1);
    tela.print("      [OK]      ");

    if(debounceBotao()){
    submenu = 'c';
    millisExpira = millisAtual;
    telaMudou = 1;
    }
    break;

    case 'c':
    medir();
    tela.setCursor(0, 0);
    tela.print("Valor atual:");
    tela.setCursor(0, 1);
    tela.print(peso);
    
    if(millisAtual - millisExpira > CALIB_MS){

      if(maiorPeso > PESO_ESTIMADO){
      submenu = 'd';
      telaMudou = 1;
      }

      else{
      submenu = 'e';
      telaMudou = 1;
      }
        
    }
    
    break;

    case 'd':
    tela.setCursor(0, 0);
    tela.print("Calib. concluida");
    tela.setCursor(0, 1);
    tela.print("      [OK]      ");

    if(debounceBotao()){
    submenu = 'a';
    menu = 'e';
    telaMudou = 1;
    }
    
    break;

    case 'e':
    tela.setCursor(0, 0);
    tela.print("Erro! Verif cabo");
    tela.setCursor(0, 1);
    tela.print("    [Voltar]    ");

    if(debounceBotao()){
    submenu = 'a';
    telaMudou = 1;
    }

    break;

  }

  }
  

void espera(){

  tela.setCursor(0, 0);
  tela.print("Pronto p/ medir");
  tela.setCursor(0, 1);
  tela.print("   [Iniciar]   ");

  if(debounceBotao()){
    menu = 't';
    submenu = 'a';
    telaMudou = 1;
  }
  
}

void trabalhando(){

  switch(submenu){

    case 'a':
    medir();
    escrever();
    
    tela.setCursor(0, 0);
    tela.print("A:      M:      ");
    
    tela.setCursor(3, 0);
    tela.print(peso/1000);

    tela.setCursor(12, 0);
    tela.print(maiorPeso/1000);
    
    
    tela.setCursor(0, 1);
    tela.print("   [Terminar]   ");

    if(debounceBotao()){
    submenu = 'b';
    telaMudou = 1;
    }
    
    break;

    case 'b':
    tela.setCursor(0, 0);
    tela.print("Salvo:");
    tela.setCursor(7, 0);
    tela.print(nomeConcat);
    tela.setCursor(0, 1);
    tela.print("      [OK]      ");

    finaliza();

    if(debounceBotao()){
    menu = 'e';
    telaMudou = 1;
    }
    
    break;

  }
  
}

void medir(){

    peso = celulaCarga.get_units();

    if (peso < 0.00)
      peso = 0.00;
  

    if (peso > maiorPeso)
    maiorPeso = peso;

}

int debounceBotao(){

if(digitalRead(PINO_BOTAO)){
  botaoApertado = millis();

  if((botaoApertado - ultimoBotaoApertado )> DEBOUNCE_MS){
    ultimoBotaoApertado = botaoApertado;
    return 1;
   }
  else{
    return 0;
  }
  }
  return 0;
}


void escrever(){

  stringDados += millisAtual;
  stringDados += ",";
  stringDados += peso;
  stringDados += ",";
  stringDados += maiorPeso;
  stringDados += "\n";
  
  if(sizeof(stringDados) == TAMANHO_BUFFER){
  arquivoLog.print(stringDados);
  arquivoLog.flush();
  stringDados = "";
  }

  
}

void finaliza(){
  arquivoLog.close();
}
