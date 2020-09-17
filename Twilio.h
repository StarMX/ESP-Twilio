#ifndef twilio_h
#define twilio_h


class Twilio
{
private:
    String get_auth_header(const String &user, const String &password);
    unsigned char h2int(char c);
    String urldecode(String str);
    String urlencode(String str);
    const char *account;
    const char *authtoken;
    const char *from;
    
public:
    Twilio(/* args */);
    ~Twilio();
    void config(const char* account,const char* authtoken,const char* from);
    void sendSms(const char* to,const char* message);
};

extern Twilio twilio;
#endif