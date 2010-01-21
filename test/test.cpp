#define FRACT_CHECKS_WITH_EXCEPTIONS
#include "../fixedpoint.h"
#include <QTest>
#include <QDebug>

#define OVF(x) ({ \
    bool _thrown = false; \
    try { x; } \
    catch (FractOverflowError&) { _thrown = true; } \
    QVERIFY(_thrown); \
})

#define NOT_OVF(x) ({ \
    bool _thrown = false; \
    try { x; } \
    catch (FractOverflowError&) { _thrown = true; } \
    QVERIFY(!_thrown); \
})

#define DOM(x) ({ \
    bool _thrown = false; \
    try { x; } \
    catch (FractDomainError&) { _thrown = true; } \
    QVERIFY(_thrown); \
})

 namespace QTest {
     template<> char *toString(const std::string& x)
     { return qstrdup(x.c_str()); }

     template<> char *toString(const Fract<16,16>& x)
     { return toString(x.toString(-1, true) + " " + x.toHex()); }
     template<> char *toString(const Fract<32,32>& x)
     { return toString(x.toString(-1, true) + " " + x.toHex()); }
     template<> char *toString(const Fract<4,12>& x)
     { return toString(x.toString(-1, true) + " " + x.toHex()); }
     template<> char *toString(const Fract<8,24>& x)
     { return toString(x.toString(-1, true) + " " + x.toHex()); }
}

class TestFixed : public QObject
{
    Q_OBJECT

private slots:
    void constructors(void)
    {
        Fract<16,16> f1(2);
        QCOMPARE((int)sizeof(f1), 4);
        QCOMPARE((unsigned)f1.floor(), 2U);
        QCOMPARE((unsigned)f1.ceil(), 2U);

        Fract<16,16> f2(2.75f);
        QCOMPARE((unsigned)f2.floor(), 2U);
        QCOMPARE((unsigned)f2.ceil(), 3U);
        QCOMPARE(f2.toFloat(), 2.75f);

        Fract<16,16> f3(2.75);
        QCOMPARE((unsigned)f3.floor(), 2U);
        QCOMPARE((unsigned)f3.ceil(), 3U);
        QCOMPARE(f3.toDouble(), 2.75);

        Fract<32,32> f4(2.75);
        QCOMPARE((unsigned)f4.floor(), 2U);
        QCOMPARE((unsigned)f4.ceil(), 3U);
        QCOMPARE(f4.toDouble(), 2.75);

        Fract<32,32> f5(-2.75);
        QCOMPARE((unsigned)f5.floor(), -3U);
        QCOMPARE((unsigned)f5.ceil(), -2U);
        QCOMPARE(f5.toDouble(), -2.75);
    }

    void constructors_overflow(void)
    {
        typedef Fract<16,16> Fract16;
        OVF(Fract16(1E+20));

        typedef Fract<1,7> Fract1;
        NOT_OVF(Fract1(0));
        NOT_OVF(Fract1(-1));
        OVF(Fract1(1));
        OVF(Fract1(2));
        OVF(Fract1(-2));

        typedef Fract<2,6> Fract2;
        NOT_OVF(Fract2(-2));
        NOT_OVF(Fract2(-1));
        NOT_OVF(Fract2(0));
        NOT_OVF(Fract2(1));
        OVF(Fract2(2));
        OVF(Fract2(-3));

        typedef Fract<8,0> FractFull;
        NOT_OVF(FractFull(0));
        NOT_OVF(FractFull(127));
        NOT_OVF(FractFull(-128));
        OVF(FractFull(128));
        OVF(FractFull(-129));
    }

    void sqroot(void)
    {
        typedef Fract<8,24> F;
        typedef Fract<4,12> FH;
        QFETCH(int, input);
        QFETCH(double, result);
        QCOMPARE(sqrt(F(input)), F(result));
        QCOMPARE(sqrt_fast(F(input)), FH(result));
    }
    void sqroot_data(void)
    {
        QTest::addColumn<int>("input");
        QTest::addColumn<double>("result");
        for (int i=2;i<100;i++)
            QTest::newRow(QString::number(i).toAscii()) << i << sqrt(double(i));
    }

    void square_root_sidecases(void)
    {
        typedef Fract<16,16> F;
        QVERIFY(sqrt(F(1)) == 1);
        QVERIFY(sqrt(F(0)) == 0);
        DOM(sqrt(F(-1)));
    }

    void tostring(void)
    {
        typedef Fract<16,16> F;
        QCOMPARE(F(12.75).toString(2), std::string("12.75"));
        QCOMPARE(F(12.75).toString(6, true), std::string("12.750000"));

        typedef Fract<32,32> F2;
        QCOMPARE(F(12.75).toString(2), std::string("12.75"));
        QCOMPARE(F(12.75).toString(6, true), std::string("12.750000"));
    }

    void addsub(void)
    {
        typedef Fract<32,32> F;
        typedef Fract<16,16> FS;
        QCOMPARE(F(11.4467) + F(740.1149), F(11.4467+740.1149));
        QCOMPARE(F(11.4467) - F(740.1149), F(11.4467-740.1149));
        QCOMPARE(F(11.25) + FS(740.75), F(11.25+740.75));
        QCOMPARE(F(11.25) - FS(740.75), F(11.25-740.75));
    }

    void parseloop(void)
    {
        QFETCH(double, result);

        typedef Fract<16,16> F;
        typedef Fract<32,32> F2;

        // TODO: Is it possible to achieve better precision?
        // Is there something fundamentally wrong with this test?
        // Anyway, this precision is enough for our use-cases.
        F f1a = F(result);
        F f1b = F::fromString(f1a.toString());
        QVERIFY(F::error(f1a, f1b) < 3);

        F2 f2a = F2(result);
        F2 f2b = F2::fromString(f2a.toString());
        QVERIFY(F2::error(f2a, f2b) < 3);
    }

    void parseloop_data(void)
    {
        QTest::addColumn<double>("result");

        QTest::newRow("basic1") << 123.339981068;
        QTest::newRow("basic2") << -123.339981068;
        QTest::newRow("basic3") << 456.478913289;
        QTest::newRow("basic4") << 999.000009999;
        QTest::newRow("basic5") << 100.0;
        QTest::newRow("basic7") << -100.0;
        QTest::newRow("basic8") << .456;
    }

    void weirdparse(void)
    {
        QFETCH(QString, string);
        QFETCH(double, result);

        typedef Fract<16,16> F;
        typedef Fract<32,32> F2;

        QCOMPARE(F::fromString(string.toStdString()), F(result));
        QCOMPARE(F2::fromString(string.toStdString()), F2(result));
    }

    void weirdparse_data(void)
    {
        QTest::addColumn<QString>("string");
        QTest::addColumn<double>("result");

        // WARNING: only use number with exact fixed point and floating point representation
        QTest::newRow("weird1") << "123" << 123.0;
        //QTest::newRow("weird2") << "123." << 123.0;
        //QTest::newRow("weird3") << "-123." << -123.0;
        //QTest::newRow("weird4") << "123.0000" << 123.0;
        //QTest::newRow("weird5") << ".0" << 0.0;
    }

    void inverse(void)
    {
        typedef Fract<2,30> F;

        qWarning() << F(detail::inverse(22194), 31).toString().c_str();
    }
};

QTEST_MAIN(TestFixed)
#include "test.moc"
