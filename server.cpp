#include <iostream>
#include <string>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int analyzePassword(string pass)
{
    int score = 0;
    bool hasUpper = false, hasLower = false;
    bool hasDigit = false, hasSpecial = false;

    int length = pass.length();

    if(length >= 8) score += 20;
    if(length >= 12) score += 10;

    for(int i = 0; i < length; i++)
    {
        if(pass[i] >= 'A' && pass[i] <= 'Z') hasUpper = true;
        else if(pass[i] >= 'a' && pass[i] <= 'z') hasLower = true;
        else if(pass[i] >= '0' && pass[i] <= '9') hasDigit = true;
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

int main()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    bind(server_socket, (sockaddr*)&server, sizeof(server));
    listen(server_socket, 5);

    cout << "Server running at http://localhost:8080\n";

    while(true)
    {
        SOCKET client_socket = accept(server_socket, NULL, NULL);

        char buffer[30000] = {0};
        recv(client_socket, buffer, 30000, 0);

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
                "<html><body>"
                "<h2>Password Strength Checker</h2>"
                "<form method='GET'>"
                "Enter Password: <input type='text' name='password'/>"
                "<input type='submit' value='Check'/>"
                "</form>"
                "</body></html>";
        }
        else
        {
            int score = analyzePassword(password);
            string level = strengthText(score);

            html =
                "<html><body>"
                "<h2>Password Strength Checker</h2>"
                "<p>Score: " + to_string(score) + "/100</p>"
                "<p>Strength: <b>" + level + "</b></p>"
                "<a href='/'>Check another password</a>"
                "</body></html>";
        }

        string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n\r\n" + html;

        send(client_socket, response.c_str(), response.length(), 0);
        closesocket(client_socket);
    }

    WSACleanup();
    return 0;
}
