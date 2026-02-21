#include <iostream>
#include <string>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstdlib>

using namespace std;

int analyzePassword(string pass)
{
    int score = 0;
    bool hasUpper = false, hasLower = false;
    bool hasDigit = false, hasSpecial = false;

    if(pass.length() >= 8) score += 20;
    if(pass.length() >= 12) score += 10;

    for(char c : pass)
    {
        if(isupper(c)) hasUpper = true;
        else if(islower(c)) hasLower = true;
        else if(isdigit(c)) hasDigit = true;
        else hasSpecial = true;
    }

    if(hasUpper) score += 15;
    if(hasLower) score += 15;
    if(hasDigit) score += 15;
    if(hasSpecial) score += 15;

    if(pass != "123456" && pass != "password")
        score += 10;

    return score;
}

string strengthText(int score)
{
    if(score <= 30) return "VERY WEAK";
    else if(score <= 50) return "WEAK";
    else if(score <= 75) return "MODERATE";
    else return "STRONG";
}

string strengthColor(int score)
{
    if(score <= 30) return "red";
    else if(score <= 50) return "orange";
    else if(score <= 75) return "#f1c40f";
    else return "green";
}

int main()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;

    int port = stoi(getenv("PORT"));
    server.sin_port = htons(port);

    bind(server_socket, (sockaddr*)&server, sizeof(server));
    listen(server_socket, 5);

    while(true)
    {
        int client_socket = accept(server_socket, NULL, NULL);

        char buffer[30000] = {0};
        read(client_socket, buffer, 30000);

        string request(buffer);
        string password = "";

        size_t pos = request.find("password=");
        if(pos != string::npos)
        {
            password = request.substr(pos + 9);
            size_t endPos = password.find(" ");
            if(endPos != string::npos)
                password = password.substr(0, endPos);
        }

        string html;

        if(password == "")
        {
            html =
            "<!DOCTYPE html>"
            "<html><head>"
            "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
            "<title>Password Strength Checker</title>"
            "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.0/css/all.min.css'>"
            "<style>"
            "body{margin:0;font-family:Arial;background:linear-gradient(135deg,#667eea,#764ba2);"
            "height:100vh;display:flex;justify-content:center;align-items:center;}"
            ".card{background:white;padding:40px;border-radius:15px;width:400px;"
            "box-shadow:0 15px 30px rgba(0,0,0,0.2);text-align:center;}"
            "h2{margin-bottom:25px;}"
            ".input-box{position:relative;}"
            "input{width:100%;padding:12px 45px 12px 12px;font-size:16px;"
            "border-radius:8px;border:1px solid #ccc;box-sizing:border-box;}"
            ".input-box i{position:absolute;right:15px;top:50%;"
            "transform:translateY(-50%);cursor:pointer;color:#666;}"
            "button{margin-top:20px;padding:10px 20px;border:none;"
            "border-radius:8px;background:#667eea;color:white;font-size:16px;cursor:pointer;}"
            "button:hover{background:#5a67d8;}"
            "</style>"
            "</head><body>"
            "<div class='card'>"
            "<h2>Password Strength Checker</h2>"
            "<form method='GET'>"
            "<div class='input-box'>"
            "<input type='password' id='password' name='password' placeholder='Enter password' required>"
            "<i class='fa-solid fa-eye' onclick='togglePassword()'></i>"
            "</div>"
            "<button type='submit'>Check Strength</button>"
            "</form>"
            "</div>"
            "<script>"
            "function togglePassword(){"
            "var x=document.getElementById('password');"
            "if(x.type==='password'){x.type='text';}"
            "else{x.type='password';}"
            "}"
            "</script>"
            "</body></html>";
        }
        else
        {
            int score = analyzePassword(password);
            string level = strengthText(score);
            string color = strengthColor(score);

            html =
            "<!DOCTYPE html>"
            "<html><head>"
            "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
            "<title>Result</title>"
            "<style>"
            "body{margin:0;font-family:Arial;background:linear-gradient(135deg,#667eea,#764ba2);"
            "height:100vh;display:flex;justify-content:center;align-items:center;}"
            ".card{background:white;padding:40px;border-radius:15px;width:400px;"
            "box-shadow:0 15px 30px rgba(0,0,0,0.2);text-align:center;}"
            "a{display:inline-block;margin-top:20px;text-decoration:none;color:#667eea;}"
            "</style>"
            "</head><body>"
            "<div class='card'>"
            "<h2>Password Strength Checker</h2>"
            "<p><b>Score:</b> " + to_string(score) + "/100</p>"
            "<p><b>Strength:</b> <span style='color:" + color + ";font-weight:bold;'>" + level + "</span></p>"
            "<a href='/'>Check another password</a>"
            "</div>"
            "</body></html>";
        }

        string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n" + html;

        send(client_socket, response.c_str(), response.length(), 0);
        close(client_socket);
    }

    close(server_socket);
    return 0;
}
