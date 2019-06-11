

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
#define TAXA_ATUALIZ 100
#define TAXA_TELA 100
#define DEBOUNCE_MS 300
#define CALIB_MS 5000
#define PESO_ESTIMADO 250
#define DELTA_RUIDO 1500

const int TELA_RS = 2;
const int TELA_EN = 3;
const int TELA_D4 = 4;
const int TELA_D5 = 5;
const int TELA_D6 = 6;
const int TELA_D7 = 7;

const int PINO_SD_CS = 10;

#define PINO_BOTAO A5
#define TAMANHO_BUFFER 512

//Variáveis do código, não mexer
char  menu = 'i';
char  submenu = 'a';

int   peso = 0;
int   maiorPeso = 0;
int   amostraAtual = 0;
int   amostraAnterior = 0;

int   millisAtual, millisAnterior;
int   millisTela;
int   millisBotao;
int   millisExpira;

bool botaoApertado;

File arquivoLog;
char nomeBase[] = "BESR";
char nomeConcat[12];
String stringDados;

HX711 celulaCarga;
const int LOADCELL_DOUT_PIN = A0;
const int LOADCELL_SCK_PIN = A1;
const long LOADCELL_OFFSET = 50682624;
const long LOADCELL_DIVIDER = 5895655;

LiquidCrystal tela(TELA_RS, TELA_EN, TELA_D4, TELA_D5, TELA_D6, TELA_D7);

bool debounceBotao();
void medir();
void escrever();
void finaliza();
void espera();

void setup() {

  //Setup do display e inicialização
  
  tela.begin(16,2);
  
  //Setup da célula de carga, sua inicialização é feita posteriormente
  celulaCarga.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  celulaCarga.set_scale(LOADCELL_DIVIDER);
  celulaCarga.set_offset(LOADCELL_OFFSET);
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
        break;
    
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
  if(!celulaCarga.wait_ready_timeout(500))
  submenu = 'c';

  if(submenu == 'a'){
  menu = 'c';
  submenu = 'a';
  }
}

void calibra(){

  switch(submenu){

    case 'a':
    tela.setCursor(0, 0);
    tela.print("Calibrar o banco");
    tela.setCursor(1, 0);
    tela.print("   [Calibrar]   ");

    if(debounceBotao)
    submenu = 'b';
    
    break;

    case 'b':
    tela.setCursor(0, 0);
    tela.print("Posicione o peso");
    tela.setCursor(1, 0);
    tela.print("      [OK]      ");

    if(debounceBotao){
    submenu = 'c';
    millisExpira = millisAtual;
    }
    break;

    case 'c':
    medir();
    tela.setCursor(0, 0);
    tela.print("Valor atual:");
    tela.setCursor(1, 0);
    tela.print(peso);
    
    if(millisAtual - millisExpira > CALIB_MS){

      if(maiorPeso > PESO_ESTIMADO)
      submenu = 'd';
      else
      submenu = 'e';
        
    }
    
    break;

    case 'd':
    tela.setCursor(0, 0);
    tela.print("Calib. concluida");
    tela.setCursor(1, 0);
    tela.print("      [OK]      ");

    if(debounceBotao){
    submenu = 'a';
    menu = 'e';
    }
    
    break;

    case 'e':
    tela.setCursor(0, 0);
    tela.print("Erro! Verif cabo");
    tela.setCursor(1, 0);
    tela.print("    [Voltar]    ");

    if(debounceBotao)
    submenu = 'a';
    
    break;

  }

  }
  

void espera(){

  tela.setCursor(0, 0);
  tela.print("Pronto p/ medir");
  tela.setCursor(1, 0);
  tela.print("   [Iniciar]   ");

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
    
    tela.setCursor(0, 0);
    tela.print("A:      M:      ");
    
    tela.setCursor(0, 3);
    tela.print(peso/100);

    tela.setCursor(0, 12);
    tela.print(maiorPeso/100);
    
    
    tela.setCursor(1, 0);
    tela.print("   [Terminar]   ");

    if(debounceBotao)
    submenu = 'b';
    
    break;

    case 'b':
    tela.setCursor(0, 0);
    tela.print("Salvo:");
    tela.setCursor(0, 7);
    tela.print(nomeConcat);
    tela.setCursor(1, 0);
    tela.print("      [OK]      ");

    finaliza();

    if(debounceBotao)
    menu = 'e';
    
    break;

  }
  
}

void medir(){

    amostraAtual = celulaCarga.get_units(10);

    if(abs(amostraAnterior - amostraAtual) > DELTA_RUIDO){
      
    peso = amostraAtual;
    
    if(peso > maiorPeso)
    maiorPeso = peso;

    amostraAnterior = amostraAtual;
    }

}

bool debounceBotao(){

  if(digitalRead(PINO_BOTAO))
    botaoApertado = 1;

  if((botaoApertado == 1) && (millisAtual - millisBotao > DEBOUNCE_MS)){
    return 1;
    botaoApertado = 0;

  }
  else
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
}
