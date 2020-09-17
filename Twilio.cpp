#include <Arduino.h>
#include "Twilio.h"

#include <base64.h>

#ifdef ESP32
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#endif
#ifdef ESP8266
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <BearSSLHelpers.h>
#endif

const char* ca_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
"-----END CERTIFICATE-----\n" \
;

Twilio twilio;
Twilio::Twilio(/* args */)
{
}

Twilio::~Twilio()
{
}
void Twilio::config(const char* account,const char* authtoken,const char* from){
    this->account= account;
    this->authtoken = authtoken;
    this->from = from;
}
void Twilio::sendSms(const char *to, const char *message)
{
    String post_data = "To=" + urlencode(to) + "&From=" + urlencode(from) + "&Body=" + urlencode(message);
    Serial.println(post_data);
#ifdef ESP32
    WiFiClientSecure *client = new WiFiClientSecure;
    client->setCACert(ca_cert);
#endif
#ifdef ESP8266
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setTrustAnchors(new BearSSL::X509List(ca_cert));
#endif

    HTTPClient http;
    http.begin(*client, "https://api.twilio.com/2010-04-01/Accounts/" + String(account) + "/Messages.json");
    http.setUserAgent("esp8266");
    http.addHeader("Authorization", get_auth_header(account, authtoken), true);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
    int httpCode = http.POST(post_data);
    if (httpCode == 201)
        Serial.print("Sent message successfully!");
    else
        Serial.printf("Sent message httpCode! %d", httpCode);
    Serial.println();
    http.end();
}

String Twilio::get_auth_header(const String &user, const String &password)
{
    size_t toencodeLen = user.length() + password.length() + 2;
    char toencode[toencodeLen];
    memset(toencode, 0, toencodeLen);
    snprintf(
        toencode,
        toencodeLen,
        "%s:%s",
        user.c_str(),
        password.c_str());

    String encoded = base64::encode((uint8_t *)toencode, toencodeLen - 1);
    String encoded_string = String(encoded);
    std::string::size_type i = 0;

    // Strip newlines (after every 72 characters in spec)
    while (i < encoded_string.length())
    {
        i = encoded_string.indexOf('\n', i);
        if (i == -1)
        {
            break;
        }
        encoded_string.remove(i, 1);
    }
    return "Basic " + encoded_string;
}

unsigned char Twilio::h2int(char c)
{
    if (c >= '0' && c <= '9')
    {
        return ((unsigned char)c - '0');
    }
    if (c >= 'a' && c <= 'f')
    {
        return ((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <= 'F')
    {
        return ((unsigned char)c - 'A' + 10);
    }
    return (0);
}

String Twilio::urldecode(String str)
{
    String encodedString = "";
    char c;
    char code0;
    char code1;
    for (unsigned int i = 0; i < str.length(); i++)
    {
        c = str.charAt(i);
        if (c == '+')
        {
            encodedString += ' ';
        }
        else if (c == '%')
        {
            i++;
            code0 = str.charAt(i);
            i++;
            code1 = str.charAt(i);
            c = (h2int(code0) << 4) | h2int(code1);
            encodedString += c;
        }
        else
        {
            encodedString += c;
        }

        yield();
    }

    return encodedString;
}

String Twilio::urlencode(String str)
{
    String encodedString = "";
    char c;
    char code0;
    char code1;
    for (unsigned int i = 0; i < str.length(); i++)
    {
        c = str.charAt(i);
        if (c == ' ')
        {
            encodedString += '+';
        }
        else if (isalnum(c))
        {
            encodedString += c;
        }
        else
        {
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9)
            {
                code1 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9)
            {
                code0 = c - 10 + 'A';
            }
            encodedString += '%';
            encodedString += code0;
            encodedString += code1;
        }
        yield();
    }
    return encodedString;
}
