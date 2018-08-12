#include "manualquery.h"
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "gw2api.h"

QString indentStr(size_t value) {
    QString result;
    result.resize(static_cast<int>(value*4));
    for(auto i = 0; i < value*4; i++) {
        result[i] = ' ';
    }

    return result;
}

QString recursiveParseArray(QString name, QJsonArray a, size_t indent = 0);
QString recursiveParseObject(QString name, QJsonObject o, size_t indent = 0);


QString parseValue(QString name, QJsonValue v, size_t indent = 0)
{
    switch(v.type()) {
    case QJsonValue::Type::Double:
        return indentStr(indent)+QString("double %1;\n").arg(name);
    case QJsonValue::Type::String:
        return indentStr(indent)+QString("QString %1;\n").arg(name);
    case QJsonValue::Type::Bool:
        return indentStr(indent)+QString("bool %1;\n").arg(name);
    case QJsonValue::Type::Object:
        return recursiveParseObject(name,v.toObject(),indent);
    case QJsonValue::Type::Array:
        return recursiveParseArray(name,v.toArray(),indent);
    default:
        break;
    }

    return QString("// probably a null type: ") += name;
}

QString recursiveParseObject(QString name, QJsonObject o, size_t indent)
{
    QString result = indentStr(indent)+QString("class %2 {\n").arg(name);

    for (auto key : o.keys()) {
        result += parseValue(key, o[key],indent+1);
    }

    result += indentStr(indent)+"};\n";
    return result;
}
QString recursiveParseArray(QString name, QJsonArray a, size_t indent)
{
    QString type = "QVariant";
    QString result;

    if (!a.empty()) {
        QList<QJsonValue::Type> types;
        QList<QString> objectTypes;
        int arrayCounter = 0;
        for (auto element : a) {
            auto elementName = QString("%1ArrayType%2").arg(name).arg(arrayCounter);
            //if (element.isObject()) {
                auto t = parseValue(QString("%1ArrayType%2").arg(name).arg(arrayCounter),element,indent);
                if (!objectTypes.contains(t)) {
                    objectTypes.push_back(t);
                    result += t;
                }
            //}
            arrayCounter++;
        }
    }
    result += indentStr(indent)+QString("QVector<%1> %2;\n").arg(type).arg(name);

    return result;
}

QString generateClasses(QByteArray data)
{
    auto doc = QJsonDocument::fromJson(data);

    if (doc.isObject()) {
        return recursiveParseObject("NameMe", doc.object());
    }
    else if (doc.isArray()) {
        return recursiveParseArray("NameMe", doc.array());
    }

    return "";
}

ManualQuery::ManualQuery(QWidget *parent) : QWidget(parent)
{
    setWindowFlag(Qt::Window, true);
    setAttribute(Qt::WA_DeleteOnClose);

    QWidget* input = new QWidget(this);
    input->setMaximumHeight(200);
    QHBoxLayout *inputLayout = new QHBoxLayout(input);
    QLineEdit* query = new QLineEdit(input);
    QPushButton* button = new QPushButton(input);
    button->setText(tr("Send"));
    inputLayout->addWidget(query);
    inputLayout->addWidget(button);
    input->setLayout(inputLayout);

    QTextEdit* result = new QTextEdit;
    result->setMinimumHeight(200);
    QTextEdit* cppResult = new QTextEdit;
    cppResult->setMinimumHeight(200);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(input);
    layout->addWidget(result);
    layout->addWidget(cppResult);
    setLayout(layout);


    //callbacks
    connect(query,&QLineEdit::returnPressed, button, &QPushButton::click);
    connect(button,&QPushButton::pressed,[=]()
    {
        (void)GW2::Api::getApi()->get(query->text(),false,[=](QByteArray data) {
            result->setText(QString::fromUtf8(data));
            cppResult->setText(generateClasses(data));
        });
    });
}
