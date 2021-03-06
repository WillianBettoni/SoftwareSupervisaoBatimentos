//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#include <vector>
#include <string.h>

#pragma hdrstop

#include "UComunicacaoSerial.h"
#include "SerialComPort.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#define constProc 1.5;

TFSerialPort *FSerialPort;
SerialCommPort *PortaSerial;

//Declara??o das Threads de processamento.
Thread *ProcessaGrafico;

// Declara??o de vari?veis de manipula??o do gr?fico chart.
unsigned int max_tela = 100;                // N?mero m?ximo de amostras do eixo X no gr?fico.
unsigned int posicao_do_grafico = 0;        // Posi??o no gr?fico para plotar a amostra lida.

// Demais vari?veis auxiliares.
float tensao = 0;
int valor = 0;
AnsiString saida;
std::vector <double> dados;

//---------------------------------------------------------------------------
double media(std::vector <double> x)
{
  //------------------------------------------------------------------------//
  // Classe.: TProcessamentoSinais                                          //
  // M?todo.: mean() - Calcula a M?dia dos Valores de um Vetor.             //
  // Autor..: Geovani Rodrigo Scolaro.                                      //
  // Data...: Novembro/2005.                                                //
  // Entrada: Vetor x[] cont?m o sinal de entrada a ser analisado.          //
  // Saida..: Retorno do valor da m?dia do vetor x[].                       //
  //------------------------------------------------------------------------//

  double soma = 0;
  double aux = 0;

  //C?lculo da m?dia  do vetor de entrada.
  for (int a = 0; a < (int) x.size(); a++)
  {
	soma += x[a];
  }

  aux = (1.0 / (int) x.size() * soma);

  //Retorna o vetor de dados calculados.
  return (aux);
}

//---------------------------------------------------------------------------

double TFSerialPort::Mediana(std::vector <double> v){
	double mediana = 0;
	mediana = v[v.size()/2-1];
	return mediana;
}

//---------------------------------------------------------------------------

double TFSerialPort::DesvioPadrao(std::vector<double> v)
{
	return sqrt(Variancia(v));
}

//---------------------------------------------------------------------------

double TFSerialPort::Variancia(std::vector<double> v)
{
  	 int tamanho = v.size();

	 if(tamanho > 0){
		 double varianciaAux = 0;
		 double t = v[0];

		 for (int i = 1; i < tamanho; i++)
		 {
			  t += v[i];
			  double diff = ((i + 1) * v[i]) - t;
			  varianciaAux += (diff * diff) / ((i + 1.0) * i);
		 }

		 return varianciaAux / (tamanho - 1);
	 }
}

//---------------------------------------------------------------------------

double TFSerialPort::Minimo(std::vector<double> v)
{
	float minimoAux = dados[1];

	for(int i = 0; i < v.size(); i++){
		if(minimoAux > dados[i]){
			minimoAux = dados[i];
		}
	};

	return minimoAux;
}

//---------------------------------------------------------------------------

double TFSerialPort::Maximo(std::vector<double> v)
{
	float maximoAux = dados[1];

	for(int i =0; i < v.size(); i++){
		if(maximoAux < dados[i]){
			maximoAux = dados[i];
		}
	}

	return maximoAux;
}

//---------------------------------------------------------------------------

double TFSerialPort::CoeficienteVariacao()
{
	 double mediaAux = media(dados);
	 double desvioAux = DesvioPadrao(dados) + constProc;
	 double cv = (desvioAux/mediaAux); //Se quiser saber em porcentagem (* 100)

	 return cv;
}

//---------------------------------------------------------------------------

double TFSerialPort::Moda(std::vector <double> v){
	int cont[v.size()];

	double conta = 0;
	double moda = 0;

	for(int i=0; i < v.size(); i++){

		for(int j=i+1; j < v.size(); j++){

			if(v[i]==v[j]){
				cont[i]++;

				if(cont[i]>conta){
					conta=cont[i];
					moda=v[i];
				}
			}

		}
		cont[i]=0;
	}

	if(conta == 0){
		return 0;
	}
	else{
		return moda;
	}

}

//---------------------------------------------------------------------------

__fastcall TFSerialPort::TFSerialPort(TComponent* Owner)
	: TForm(Owner)
{

}

//---------------------------------------------------------------------------

void __fastcall TFSerialPort::FormCreate(TObject *Sender)
{
	//------- Gr?fico de sinais ---------------------------------------------

	// Redimensiona o valor m?ximo do eixo x com o tamanho da tela desejada.
	Chart1->BottomAxis->Maximum = max_tela;
	dados.resize(max_tela+1);

	// Expande o gr?fico para comportar a quantidade de amostras contidas em max_tela.
	for (unsigned int a = 0; a < max_tela+1; a++)
	{
		Chart1->Series[0]->AddY(0);
		dados.at(a) = 0;
	}

	// Atualiza o chart.
	Chart1->Refresh();

	//------- Gr?fico de sinais ---------------------------------------------

	// Verifica quais portas seriais est?o dispon?veis/conectadas.
	std::vector <AnsiString> asDetectedPorts;
	asDetectedPorts = PortaSerial->List();

	CbSerialPort1->Clear();
	for (unsigned int a = 0; a < asDetectedPorts.size(); a++)
	{
		CbSerialPort1->Items->Add(asDetectedPorts[a]);
	}

	CbSerialPort1->ItemIndex = 0;

	//Alinhar o edit de batimentos
	eBatimentos->Left = BtClosePort->Left;
	eBatimentos->Width = BtClosePort->Width;

	pnlTextos->Left = BtClosePort->Left;
	lblTituloPrecisao->Left = BtClosePort->Left;
	lblStatus->Left = BtClosePort->Left;
}

void TFSerialPort::coracaoBatendo(bool bater){
	((TGIFImage*)(imgCoracao->Picture->Graphic))->Animate = bater;
	((TGIFImage*)(imgCoracao->Picture->Graphic))->AnimationSpeed= 100;
}

//---------------------------------------------------------------------------

void __fastcall TFSerialPort::BtOpenPortClick(TObject *Sender)
{
	// Configura??es da Porta Serial selecionada.
	PortaSerial = new SerialCommPort();
	PortaSerial->Open(CbSerialPort1->Text, CbBaudRate1->Text);

	// Declara??o da thread para leitura dos pacotes e plotagem do gr?fico.
	ProcessaGrafico = new Thread(true);
	ProcessaGrafico->Resume();

	BtOpenPort->Enabled = false;
	//Cora??o batendo
	coracaoBatendo(true);
}

//---------------------------------------------------------------------------

void __fastcall TFSerialPort::BtClosePortClick(TObject *Sender)
{
	BtOpenPort->Enabled = true;

	// Verifica??o da inst?ncia da thread de atualiza??o do gr?fico.
	if (ProcessaGrafico != NULL)
	{
		//ProcessaGrafico->WaitFor();
		ProcessaGrafico->Terminate();
		delete ProcessaGrafico;
		ProcessaGrafico = NULL;
	}

	// Verifica??o da inst?ncia da porta serial.
	if (PortaSerial != NULL)
	{
		Sleep(1000);
		PortaSerial->Close();
	}

	delete PortaSerial;
	PortaSerial = NULL;
	//Cora??o batendo
	coracaoBatendo(false);
}

//---------------------------------------------------------------------------

void __fastcall TFSerialPort::Chart1AfterDraw(TObject *Sender)
{
	MostrarBarraAtualizacao();

	Chart1->Hint = "";

	MostrarDesvioPadrao();

	MostrarVariancia();

	MostrarMediana();

	MostrarMedia();

	MostrarModa();

	MostrarCV();

	MostrarMinimo();

	MostrarMaximo();
}

//---------------------------------------------------------------------------

void TFSerialPort::AdicionarHint(double valor, String legenda)
{
	if(Chart1->Hint != ""){
		Chart1->Hint = Chart1->Hint + sLineBreak;
	}

	Chart1->Hint = Chart1->Hint + legenda + ": " + FormatFloat("#,##0.00", valor);
}

//---------------------------------------------------------------------------

void TFSerialPort::MostrarBarraAtualizacao()
{
	double xi, xf, yi, yf;

	xi = Chart1->BottomAxis->CalcPosValue(posicao_do_grafico - 1) - 15;
	yi = Chart1->LeftAxis->CalcPosValue(Chart1->LeftAxis->Minimum) - 1;
	xf = Chart1->BottomAxis->CalcPosValue(posicao_do_grafico - 1) + 15;
	yf = Chart1->LeftAxis->CalcPosValue(Chart1->LeftAxis->Maximum) + 1;

	Chart1->Canvas->Pen->Style = psSolid;
	Chart1->Canvas->Pen->Color = clWhite;
	Chart1->Canvas->Brush->Color = clWhite;
	Chart1->Canvas->Rectangle(xi,yi,xf,yf); // Desenho da barra de atualiza??o da tela.
}

//---------------------------------------------------------------------------

void TFSerialPort::MostrarDesvioPadrao()
{
	if (cbDesvio->Checked)
	{
		double desvio_padrao = DesvioPadrao(dados) + constProc;
		MostrarValor(desvio_padrao, "Desvio Padr?o", clYellow);
	}
}

//---------------------------------------------------------------------------

void TFSerialPort::MostrarCV(){
	if (cbCoeficienteVariacao->Checked)
	{
		double cv = CoeficienteVariacao();
		MostrarValor(cv, "Coeficiente Varia??o", clMoneyGreen);
	}
}

//---------------------------------------------------------------------------

void TFSerialPort::MostrarMedia()
{
	if (cbMedia->Checked)
	{
		double mediaAux = media(dados);
		MostrarValor(mediaAux, "M?dia", clFuchsia);
	}
}

//---------------------------------------------------------------------------

void TFSerialPort::MostrarMinimo()
{
	if (cbMinimo->Checked)
	{
		double minimoAux = Minimo(dados);
		MostrarValor(minimoAux, "M?nimo", clGray);
	}
}

//---------------------------------------------------------------------------

void TFSerialPort::MostrarMaximo()
{
	if (cbMaximo->Checked)
	{
		double maximoAux = Maximo(dados);
		MostrarValor(maximoAux, "M?ximo", clSilver);
	}
}

//---------------------------------------------------------------------------

void TFSerialPort::MostrarMediana()
{
	if (cbMediana->Checked)
	{
		double medianaAux = Mediana(dados);
		MostrarValor(medianaAux, "Mediana", clRed);
	}
}

//---------------------------------------------------------------------------

void TFSerialPort::MostrarVariancia()
{
	if (cbVariancia->Checked)
	{
		double varianciaAux = Variancia(dados) + constProc;
		MostrarValor(varianciaAux, "Vari?ncia", clGreen);
	}
}

//---------------------------------------------------------------------------

void TFSerialPort::MostrarModa()
{
	if (cbModa->Checked)
	{
		double modaAux = Moda(dados);
		MostrarValor(modaAux, "Moda", clAqua);
	}
}

//---------------------------------------------------------------------------

void TFSerialPort::MostrarValor(double valor, String legenda, TColor corLegenda)
{
	double xi, xf, yi, yf;

	//C?lculo das coordenadas para a Moda.
	xi = Chart1->BottomAxis->CalcXPosValue(Chart1->BottomAxis->Minimum);
	yi = Chart1->LeftAxis->CalcYPosValue(valor);
	xf = Chart1->BottomAxis->CalcXPosValue(Chart1->BottomAxis->Maximum);
	yf = Chart1->LeftAxis->CalcYPosValue(valor) + 1.0;

	//Aplica a linha demarcada.
	Chart1->Canvas->Pen->Style=psSolid;
	Chart1->Canvas->Pen->Color=corLegenda;
	Chart1->Canvas->Rectangle(xi,yi,xf,yf);

	//Aplica o valor calculado.
	Chart1->Canvas->Brush->Color=corLegenda;
	Chart1->Canvas->Pen->Color=clBlack;
	Chart1->Canvas->TextOut(xi,yi, legenda + ": " + FloatToStrF(valor,ffFixed,10,2));

	AdicionarHint(valor, legenda);
}

//---------------------------------------------------------------------------

void __fastcall TFSerialPort::AtualizaGrafico()
{
	// Plota a amostra no gr?fico.
	FSerialPort->Chart1->Series[0]->YValues->Value[posicao_do_grafico] = tensao;
	dados.at(posicao_do_grafico) = tensao;
}

//---------------------------------------------------------------------------

void __fastcall TFSerialPort::CbSerialPort1DropDown(TObject *Sender)
{
	// Verifica quais portas seriais est?o dispon?veis/conectadas.
	std::vector <AnsiString> asDetectedPorts;
	asDetectedPorts = PortaSerial->List();

	CbSerialPort1->Clear();
	for (unsigned int a = 0; a < asDetectedPorts.size(); a++)
	{
		CbSerialPort1->Items->Add(asDetectedPorts[a]);
	}

	CbSerialPort1->ItemIndex = 0;
}


//---------------------------------------------------------------------------

//---------------------------------------------------------------------------//
//                Declara??o da thread de processamento                      //
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------

__fastcall Thread::Thread(bool CreateSuspended)
				: TThread(CreateSuspended)
{
	Priority = tpNormal;
}

//---------------------------------------------------------------------------

void __fastcall Thread::Execute()
{
	AnsiString check;

	while(!Terminated)
	{
		std::vector <unsigned char> buffer1;

		int valor = 0;
		AnsiString saida;

		// Envia o buffer pela porta serial.
		PortaSerial->WriteABuffer("A", 1);

		// Instante de tempo para receber resposta do mestre.
		Sleep(30);

		// Armazena em buffer a resposta enviada pelo mestre com as temperaturas.
		buffer1 = PortaSerial->ReadBuffer();

		//------------------------------------------------------------------------
		// AQUISI??O DOS PACOTES DE DADOS DE MODO CONT?NUO.
		//------------------------------------------------------------------------

		// Verifica??o e Valida??o do pacote de dados recebido.
		if (buffer1[0] == '#' && buffer1[1] == '$' && buffer1[2] == ':')
		{
			// Verifica??o/valida??o dos pacotes.
			unsigned char checksum = 0x00;
			for(unsigned char index = 0; index < 9; index++)
			{
				checksum ^= buffer1[index];
			}


			if (buffer1[9] == checksum)
				check = "PACOTE V?LIDO";
			else
				check = "PACOTE INV?LIDO";

			// Calculo da temperatura conforme os bytes recebidos (10 bits).
			valor = (buffer1[3] << 8) + (buffer1[4]);
			tensao = (valor * 0.004887585532749);


			// ----------- ALTORITMO DE CONTAGEM DE BATIMENTOS POR MINUTO ---------/

			// A frequencia de amostragem ? controlada aqui pelo supervis?rio
			// desta forma, estamos recebendo este pacote a cada 30 ms (Sleep(30))

			// 30ms = 0.03s

			// frequ?ncia muito baixa (0 a 0,4 Hz)
			// frequ?ncia baixa (0,04 a 0,15 Hz)
			// alta frequ?ncia (0,15 a 0,40 Hz)

			// frequencia de amostragem com 30 ms
			// Fs = 1/Ts       1/0,03 =     Fs = 33.33 Hz

			desvio_padrao_bpm = FSerialPort->DesvioPadrao(dados) + constProc;
			
			if(desvio_padrao_bpm > 1.55){

				if(tensao > desvio_padrao_bpm && tensao > valorAnt && !FSerialPort->habilitarbatimento){
					FSerialPort->habilitarbatimento = true;
				}

				if(FSerialPort->cbCompensar->Checked){
					if(tensao > desvio_padrao_bpm && tensao < valorAnt && FSerialPort->habilitarbatimento){
						FSerialPort->countAux++;
					}

					if(FSerialPort->countAux == 3 && FSerialPort->habilitarbatimento){
						FSerialPort->tmp_batimento++;
						FSerialPort->habilitarbatimento = false;
						FSerialPort->countAux = 0;
					}
				} else {
                    if(tensao > desvio_padrao_bpm && tensao < valorAnt && FSerialPort->habilitarbatimento){
						FSerialPort->tmp_batimento++;
						FSerialPort->habilitarbatimento = false;
					}
				}

                valorAnt = tensao;

			} else { 
				FSerialPort->LimparVariaveisBatimentos();
			}

			// ---------------------  FIM ALGORITMO ----------------------------- //

			//------- Gr?fico de sinais ------------------------------------------

			// Verifica??o do fim do gr?fico.
			if (posicao_do_grafico == max_tela + 1)
			{
				posicao_do_grafico = 0;
			}

			// Plotagem dos dados sincronizados com a thread.
			Synchronize(FSerialPort->AtualizaGrafico);

			// Incremento da posi??o do gr?fico.
			posicao_do_grafico++;

			// Atualiza??o do chart com os novos dados.
			FSerialPort->Chart1->Refresh();
		}
		else
		{
			// Sa?da indicando erro de recebimento de pacotes.
			saida = saida + "\nErro no recebimento do pacote de dados!";
		}
	}
}

void TFSerialPort::LimparVariaveisBatimentos(){
	eBatimentos->Text = IntToStr(0);;
	lbl5segundos->Caption = "05 segundos..: " + IntToStr(0);
	lbl10segundos->Caption = "10 segundos..: " + IntToStr(0);
	lbl20segundos->Caption = "20 segundos..: " + IntToStr(0);
	lbl30segundos->Caption = "30 segundos..: " + IntToStr(0);
	lblStatus->Caption = "Sem Desvio Padr?o...";
	count = 0;
	countAux = 0;
	tmp_batimento = 0;
	soma = 0;
	habilitarbatimento = false;
}

//---------------------------------------------------------------------------

void __fastcall TFSerialPort::Sair1Click(TObject *Sender)
{
	FSerialPort->Close();
}

//---------------------------------------------------------------------------

void __fastcall TFSerialPort::cbBatimentosClick(TObject *Sender)
{
	if(!cbBatimentos->Checked){
		LimparVariaveisBatimentos();
	}
}

//---------------------------------------------------------------------------

void __fastcall TFSerialPort::tmrBpmTimer(TObject *Sender)
{
	//5 segundos
	if(FSerialPort->tmp_batimento > 0){

		float compensar = 1;

		if (cbCompensar->Checked) {
			compensar = 0.8;
		}

		float bpm = (60000 * (FSerialPort->tmp_batimento * compensar))/(tmrBpm->Interval);
		FSerialPort->tmp_batimento = 0;
		FSerialPort->lbl5segundos->Caption = "05 segundos..: " + FormatFloat("0", bpm);

		if (count==0) {
			FSerialPort->lblStatus->Caption = "Iniciando contagem...";
		}

		count++;
		soma += bpm;
	}

	//10 segundos
	if (count == 2) {
		FSerialPort->lbl10segundos->Caption = "10 segundos..: " + IntToStr(soma/count);
		FSerialPort->lblStatus->Caption = "Calibrando a m?quina...";
	}

	//20 segundos
	if (count == 4) {
		FSerialPort->lbl20segundos->Caption = "20 segundos..: " + IntToStr(soma/count);
		FSerialPort->lblStatus->Caption = "Ajustando os c?lculos...";
	}

	//30 segundos
	if (count == 6) {
		FSerialPort->lbl30segundos->Caption = "30 segundos..: " + IntToStr(soma/count);
		FSerialPort->lblStatus->Caption = "T? quase...";
	}

	//40 segundos
	if (count == 8) {
		FSerialPort->eBatimentos->Text = IntToStr(soma/count);
		FSerialPort->lblStatus->Caption = "Batimentos calculados...";
		count = 0;
		soma = 0;
	}
}
//---------------------------------------------------------------------------

