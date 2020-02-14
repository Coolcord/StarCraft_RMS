#ifndef HTML_STRING_HELPER_H
#define HTML_STRING_HELPER_H

#include <QString>
#include <QHash>

class HTML_String_Helper {
public:
    HTML_String_Helper();
    ~HTML_String_Helper();
    QString Convert_From_HTML_Name(const QString &string);

private:
    QString Get_String_From_Code(const QString &code);
    void Populate_HTML_Code_Hash();

    QHash<QString, int> *htmlCodeHash;
};

#endif // HTML_STRING_HELPER_H
