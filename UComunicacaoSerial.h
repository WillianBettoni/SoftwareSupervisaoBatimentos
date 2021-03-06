//---------------------------------------------------------------------------

#ifndef UComunicacaoSerialH
#define UComunicacaoSerialH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <VCLTee.Chart.hpp>
#include <VCLTee.Series.hpp>
#include <VclTee.TeeGDIPlus.hpp>
#include <VCLTee.TeEngine.hpp>
#include <VCLTee.TeeProcs.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Imaging.GIFImg.hpp>
//---------------------------------------------------------------------------
class TFSerialPort : public TForm
{
__published:	// IDE-managed Components
	TPanel *Panel1;
	TButton *BtClosePort;
	TButton *BtOpenPort;
	TComboBox *CbBaudRate1;
	TLabel *Label2;
	TComboBox *CbSerialPort1;
	TLabel *Label1;
	TImage *imgCoracao;
	TPanel *Panel3;
	TChart *Chart1;
	TLineSeries *Series1;
	TPanel *Panel2;
	TLabel *Label3;
	TCheckBox *cbBatimentos;
	TCheckBox *cbMedia;
	TCheckBox *cbVariancia;
	TCheckBox *cbMediana;
	TCheckBox *cbModa;
	TCheckBox *cbDesvio;
	TCheckBox *cbCoeficienteVariacao;
	TCheckBox *cbMinimo;
	TCheckBox *cbMaximo;
	TTimer *tmrBpm;
	TEdit *eBatimentos;
	TPanel *pnlTextos;
	TLabel *lbl5segundos;
	TLabel *Label4;
	TLabel *lblTitulo;
	TLabel *lbl10segundos;
	TLabel *lbl20segundos;
	TLabel *lbl30segundos;
	TLabel *lblSubtitulo;
	TLabel *lblTituloPrecisao;
	TLabel *lblStatus;
	TCheckBox *cbCompensar;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall BtOpenPortClick(TObject *Sender);
	void __fastcall Chart1AfterDraw(TObject *Sender);
	void __fastcall BtClosePortClick(TObject *Sender);
	void __fastcall CbSerialPort1DropDown(TObject *Sender);
	void __fastcall Sair1Click(TObject *Sender);
	void __fastcall cbBatimentosClick(TObject *Sender);
	void __fastcall tmrBpmTimer(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TFSerialPort(TComponent* Owner);

	void __fastcall AtualizaGrafico();
	void __fastcall LerPortaSerial();
	double Moda(std::vector <double> v);
	double Mediana(std::vector <double> v);
	double DesvioPadrao(std::vector<double> v);
	double Variancia(std::vector<double> v);
	double Maximo(std::vector<double> v);
    double Minimo(std::vector<double> v);
    double CoeficienteVariacao();
	void MostrarDesvioPadrao();
	void MostrarVariancia();
    void MostrarBarraAtualizacao();
	void MostrarMedia();
	void MostrarMediana();
	void MostrarModa();
	void MostrarCV();
	void MostrarMinimo();
    void MostrarMaximo();
	void MostrarValor(double valor, String legenda, TColor corLegenda);
	void AdicionarHint(double valor, String legenda);
	void coracaoBatendo(bool bater);
    void LimparVariaveisBatimentos();
	int tmp_batimento = 0;
	bool habilitarbatimento = false;
	int soma = 0;
	int count = 0;
	int countAux = 0;
};



//---------------------------------------------------------------------------

class Thread : public TThread
{
        private:
        protected:
                void __fastcall Execute();

        public:
			__fastcall Thread(bool CreateSuspended);
			float desvio_padrao_bpm = 0.0;
			float valorAnt = 0.0;
};

//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
extern PACKAGE TFSerialPort *FSerialPort;
//---------------------------------------------------------------------------
#endif
