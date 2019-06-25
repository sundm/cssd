#ifndef QSCANNER_H
#define QSCANNER_H

#include <QThread>

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
#  if defined( QSCANNER_LIB_STATIC )
#    define SCANNER_EXPORT
#  elif defined( QSCANNER_LIB )
#    define SCANNER_EXPORT   Q_DECL_EXPORT
#  else
#    define SCANNER_EXPORT   Q_DECL_IMPORT
#  endif
#else
#  define SCANNER_EXPORT
#endif

class SCANNER_EXPORT Scanable
{
public:
	Scanable();
	virtual ~Scanable();
	Scanable(const Scanable&) = delete;
	Scanable& operator=(const Scanable &) = delete;
	Scanable(Scanable &&) = delete;
	Scanable & operator=(Scanable &&) = delete;

	void bind();
	void setCandidate(Scanable *);
	virtual void handleBarcode(const QString &) = 0;
private:
	Scanable * _candidate = nullptr;
};


class SCANNER_EXPORT IBarcodeScanner : public QObject {
	Q_OBJECT
public:
	IBarcodeScanner(QObject *parent = Q_NULLPTR);
	virtual ~IBarcodeScanner() = default;

	virtual void start() = 0;
	virtual void stop() = 0;

protected slots:
	void handleBardode(const QString &); // call this when a barcode is read

private:
	friend class BarcodeScannerFactory;
	Scanable * _s;
};


enum class BarcodeScannerType {
	USB_HID_POS_NLOY20, // NewLand OY-20
	HID_KBW             // HID Keyboard (may be USB-HID, Serial-HID, Bluetooth-HID, etc.)
};


class SCANNER_EXPORT BarcodeScannerFactory
{
public:
	static IBarcodeScanner *activeScanner;
	static IBarcodeScanner * Create(BarcodeScannerType type, QObject *parent);

	static void setScanable(Scanable *s);
	//static bool connect(const QObject * receiver, const char * method, Qt::ConnectionType type = Qt::AutoConnection);
	//static bool connect(QObject *context, std::function<void(const QString &)> method);
};

#endif // !QSCANNER_H