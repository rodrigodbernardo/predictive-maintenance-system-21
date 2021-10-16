/*
------ CHAVES UTILIZADAS NOS FIRMWARES ------

IMPORTANTE: ESTE ARQUIVO NÃO DEVE SER ENVIADO AO GITHUB E DEVE PERMANECER 
ATUALIZADO NO GOOGLE DRIVE DO PROJETO OU OUTRO MEIO NÃO PUBLICO


UTILIZAÇÃO:

1. FAÇA O DOWNLOAD DESTE ARQUIVO PARA SUA MÁQUINA E MOVA PARA A PASTA A SER UTILIZADA
2. ADICIONE O CAMINHO DESTE ARQUIVO NO ARQUIVO .GITIGNORE, PARA ELE NAO SER ENVIADO PARA O GITHUB
3. COMENTE TODAS AS VARIAVEIS E CONSTANTES QUE NAO SERAO UTILIZADAS EM SEU PROGRAMA

ESTES PASSOS DEVEM SER REPETIDOS PARA CADA ARQUIVO DE CHAVE BAIXADO, SEM EXCLUIR OS ANTERIORES

CERTIFIQUE-SE DE QUE O ARQUIVO PRINCIPAL ESTÁ ATUALIZADO NO GOOGLE DRIVE. É PREFERIVEL MANTER APENAS
UM ARQUIVO DE CHAVES NO GOOGLE DRIVE, PARA EVITAR CONFUSÃO

*/


//
// Chaves relativas à conexão Wi-Fi
//

char * ssid[]      = {"SSID-teste","FLAVIO_02"};
char * password[]  = {"SENHA-01","8861854611"};

//
// Chaves relativas à conexão MQTT
//

const char * topico_teste_entrada       = "pibiti-ifce/topico-teste-entrada";
const char * topico_teste_saida         = "pibiti-ifce/topico-teste-saida";

const char * broker_addr        = "broker.mqtt-dashboard.com";
//const char * broker_addr        = "broker.mqttdashboard.com";

const int broker_port           = 1883;
