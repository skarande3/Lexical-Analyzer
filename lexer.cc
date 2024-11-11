/*
 * Copyright (C) Rida Bazzi
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>

#include "lexer.h"
#include "inputbuf.h"

using namespace std;

string reserved[] = {
    "END_OF_FILE",
    "IF", "WHILE", "DO", "THEN", "PRINT",
    "PLUS", "MINUS", "DIV", "MULT",
    "EQUAL", "COLON", "COMMA", "SEMICOLON",
    "LBRAC", "RBRAC", "LPAREN", "RPAREN",
    "NOTEQUAL", "GREATER", "LESS", "LTEQ", "GTEQ",
    "DOT", "NUM", "ID", "ERROR", "REALNUM", "BASE08NUM", "BASE16NUM" // TODO: Add labels for new token types here (as string)
};

#define KEYWORDS_COUNT 5
string keyword[] = {"IF", "WHILE", "DO", "THEN", "PRINT"};

void Token::Print()
{
    cout << "{" << this->lexeme << " , "
         << reserved[(int)this->token_type] << " , "
         << this->line_no << "}\n";
}

LexicalAnalyzer::LexicalAnalyzer()
{
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}

bool LexicalAnalyzer::SkipSpace()
{
    char c;
    bool space_encountered = false;

    input.GetChar(c);
    line_no += (c == '\n');

    while (!input.EndOfInput() && isspace(c))
    {
        space_encountered = true;
        input.GetChar(c);
        line_no += (c == '\n');
    }

    if (!input.EndOfInput())
    {
        input.UngetChar(c);
    }
    return space_encountered;
}

bool LexicalAnalyzer::IsKeyword(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++)
    {
        if (s == keyword[i])
        {
            return true;
        }
    }
    return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++)
    {
        if (s == keyword[i])
        {
            return (TokenType)(i + 1);
        }
    }
    return ERROR;
}

Token LexicalAnalyzer::ScanNumber()
{
    char c;
    bool flagBase08_1 = true;
    bool flagBase08_2 = true;
    bool flagBase16_1 = true;
    bool flagBase16_2 = true;
    bool needToUnget = true;
    string str = "";

    input.GetChar(c);
    if (isdigit(c))
    {
        if (c == '0') // if the character == 0
        {
            tmp.lexeme = "0";
            if (!input.EndOfInput())
            {
                input.GetChar(c); // get next char

                if (c == '.') // REALNUM (num "dot" digit digit*) // if the character == 0.
                {
                    tmp.lexeme += c;

                    if (!input.EndOfInput())
                    {
                        input.GetChar(c);
                        // cout << "after . it's prinitng a:" << c << endl;
                        if (isdigit(c)) // (num dot "digit" digit*)
                        {
                            tmp.lexeme += c;

                            if (!input.EndOfInput())
                            {
                                input.GetChar(c);

                                while (!input.EndOfInput() && isdigit(c)) //(num dot digit "digit*")
                                {
                                    tmp.lexeme += c;
                                    input.GetChar(c);
                                }
                                tmp.line_no = line_no;
                                tmp.token_type = REALNUM;
                                return tmp;
                            }
                        }
                        else // if it was just 0., unget that . to be processed as a NUM and DOT
                        {
                            input.UngetChar(c);
                            input.UngetChar('.');
                            tmp.lexeme = '0';
                        }
                    }
                }

                if (c == 'x') // BASE08NUM or BASE16NUM if the starting digit it a 0
                {
                    tmp.lexeme += c;
                    if (!input.EndOfInput())
                    {
                        input.GetChar(c);

                        if (c == '0') // if it's 0x0
                        {
                            tmp.lexeme += c;
                            if (!input.EndOfInput())
                            {
                                input.GetChar(c);

                                if (c == '8')
                                {
                                    tmp.lexeme += c;
                                    tmp.line_no = line_no;
                                    tmp.token_type = BASE08NUM;
                                    return tmp;
                                }
                                else
                                {
                                    input.UngetChar(c); //
                                    input.UngetChar('0');
                                    input.UngetChar('x');
                                    tmp.lexeme = '0';
                                }
                            }
                        }

                        else if (c == '1')
                        {
                            tmp.lexeme += c;
                            if (!input.EndOfInput())
                            {
                                input.GetChar(c);

                                if (c == '6')
                                {
                                    tmp.lexeme += c;
                                    tmp.line_no = line_no;
                                    tmp.token_type = BASE16NUM;

                                    return tmp;
                                }
                            }
                        }
                    }
                }
            }
        }
        else // starts with anything other then 0
        {
            //      cout << "c1: " << c << endl;
            tmp.lexeme = ""; // 1
            //     cout << "lexeme1: " << tmp.lexeme << endl;

            if (c < '1' || c > '7') // check for the first character to be between 1 and 7 for BASE08NUM
            {
                flagBase08_1 = false; // true
            }

            if (!((c >= 'A' && c <= 'F') || (c >= '1' && c <= '9'))) // check for the first character to be between 1 and 9 or A to F for BASE16NUM
            {
                //   cout << "inside 1st char check" << endl;
                flagBase16_1 = false; // true
            }

            tmp.lexeme += c; // 1
                             //      cout << "lexeme2: " << tmp.lexeme << endl;

            input.GetChar(c); // for this if statement                      //1A
                              //    cout << "c2: " << c << endl;

            // cout << c << "<- c" << endl;

            while (!input.EndOfInput() && isdigit(c)) // this will take all the digits
            {
                // cout << "in while 1\n";
                tmp.lexeme += c; // 11
                                 //      cout << "lexeme3: " << tmp.lexeme << endl;

                if (c < '0' || c > '7') // check for the 2nd character to the "_" to be between 0 and 7 for BASE08NUM
                {
                    flagBase08_1 = false;
                }

                input.GetChar(c); // x
                //    cout << "c3: " << c << endl;

                // cout << "c is: " << c << endl;
            }
            // cout << "lexeme: " << tmp.lexeme << endl;
            // cout << "c is: " << c << endl;
            if (c != 'x' && (c >= 'A' && c <= 'F')) // c == 'x' ||
            {
                flagBase08_1 = false; ////////////////////////
                while (!input.EndOfInput() && c != '\n' && c != 'x')
                {
                    //  cout << "flag 800\n";

                    flagBase08_1 = false;
                    str += c;
                    if (!((c >= 'A' && c <= 'F') || (c >= '0' && c <= '9'))) // check for the 2nd character to the "_" to be between 0 and 7 for BASE08NUM
                    {
                        // cout << "flag 801\n";
                        flagBase16_1 = false; // false
                        break;
                    }

                    // tmp.lexeme += c;
                    //   cout
                    //  << "flag 802 \n"
                    //     << tmp.lexeme << str << endl;
                    input.GetChar(c); // x
                    // cout << "flag 803\n";
                    // move to next input
                }
                // cout << "..c " << c << endl;
                // cout << "inpt end " << input.EndOfInput() << endl;

                if (c != 'x')
                {
                    // cout << "iside" << endl;
                    // cout << str << endl;
                    input.UngetString(str);
                    // cout << "print" << endl;
                    // input.GetChar(c);
                    //   cout << c << endl;
                    //  input.UngetChar(c);

                    // tmp.token_type = NUM;
                    //  tmp.line_no = line_no;
                    //  return tmp;
                    needToUnget = false;
                }
            }
            // cout << "flag 804\n";
            // if (c == 'x')
            // {
            //     flagBase08_2 = true;
            // }

            if (!input.EndOfInput() && c == 'x')
            {
                //   cout << "f1\n";
                if ((c == 'x' && flagBase08_1)) //|| (c == 'x' && flagBase16_1)) // BASE08NUM or BASE16NUM if the starting digit it a 0
                {
                    //  cout << "in flagbase 8\n";

                    // tmp.lexeme += c;
                    if (!input.EndOfInput())
                    {
                        input.GetChar(c); // 1
                                          //  cout << "c4: " << c << endl;

                        //      cout << "flag 808. c:" << c << endl;
                        if (c == '0') // if it's 0x0
                        {
                            // cout << "f3\n";

                            // tmp.lexeme += c;
                            if (!input.EndOfInput())
                            {
                                input.GetChar(c); //\n

                                if (c == '8')
                                {
                                    // cout << "f4\n";

                                    tmp.lexeme += "x08";
                                    tmp.line_no = line_no;
                                    tmp.token_type = BASE08NUM;
                                    return tmp;
                                }
                                // else
                                // {
                                //  cout << "flag 809\n";
                                //   cout << c << endl;
                                input.UngetChar(c);
                                input.UngetChar('0');
                                input.UngetChar('x');
                                input.UngetString(str);

                                needToUnget = false;
                                //     // tmp.lexeme = '0';
                                // }
                            }
                        }
                        else if (c == '1')
                        {
                            //  cout << "f15n";

                            // tmp.lexeme += c;
                            // cout << "lexeme4: " << tmp.lexeme << endl;

                            if (!input.EndOfInput())
                            {
                                input.GetChar(c);

                                if (c == '6')
                                {
                                    tmp.lexeme += "x16";
                                    //    cout << "lexeme5: " << tmp.lexeme << endl;

                                    tmp.line_no = line_no;
                                    tmp.token_type = BASE16NUM;
                                    // cout << "encountered 1st" << endl;
                                    return tmp;
                                }
                            }
                        }
                    }
                }
                else if ((c == 'x' && flagBase16_1))
                {
                    //  cout << "comein" << endl;
                    if (!input.EndOfInput())
                    {
                        input.GetChar(c); // 1
                                          //   cout << "flag 808. c:" << c << endl;
                        if (c == '1')     // if it's 0x0
                        {
                            // cout << "f3\n";

                            // tmp.lexeme += c;
                            if (!input.EndOfInput())
                            {
                                input.GetChar(c); // 6

                                if (c == '6')
                                {
                                    // cout << "f4\n";

                                    tmp.lexeme += str + "x16";
                                    tmp.line_no = line_no;
                                    tmp.token_type = BASE16NUM;
                                    // cout << "encountered 2st" << endl;

                                    return tmp;
                                }
                                //     cout << "flag 809\n";
                                //     cout << c << endl;
                                input.UngetChar(c);
                                input.UngetChar('1');
                                input.UngetChar('x');
                                input.UngetString(str);

                                needToUnget = false;
                            }
                        }
                        else
                        {
                            // this means the inupt is something like 123AAx09
                            // cout << "this is str: " << str << endl;
                            input.UngetChar(c); // 0
                            input.UngetChar('x');
                            input.UngetString(str);
                            needToUnget = false;
                        }
                    }
                }
                else
                {
                    input.UngetString(str);
                }
                /*else
                {
                    if (!input.EndOfInput())
                    {
                        if (c == 'x')
                        {
                            if (!input.EndOfInput())
                            {
                                input.GetChar(c);
                                if (c == '1')
                                {
                                    input.GetChar(c);
                                    if (c == '6')
                                    {

                                    }
                                }
                            }
                        }
                    }

                    input.UngetChar(c);
               */
            }

            if (!input.EndOfInput())
            {
                //  cout << "f13\n";
                if (c == '.') // if there is a dot after all the digits, REALNUM (num "dot" digit digit*)
                {
                    // tmp.lexeme += c;

                    if (!input.EndOfInput())
                    {
                        input.GetChar(c); // a

                        if (isdigit(c)) // REALNUM (num dot "digit" digit*)
                        {
                            tmp.lexeme += "." + string(1, c);

                            if (!input.EndOfInput())
                            {
                                while (!input.EndOfInput() && isdigit(c)) // this will take all the digits REALNUM (num dot digit "digit*")
                                {
                                    input.GetChar(c); // 1
                                    if (isdigit(c))
                                    {
                                        tmp.lexeme += c;
                                    }
                                }

                                if (!input.EndOfInput())
                                {
                                    input.UngetChar(c);
                                }

                                // After exiting the loop, i didn't append 'c' unless it's valid
                                tmp.token_type = REALNUM;
                                tmp.line_no = line_no;
                                return tmp;
                            }
                        }
                        else
                        {
                            // the mandatory first decimal digit isn't present. ex: 10.a or 10.
                            input.UngetChar(c);
                            input.UngetChar('.');
                            needToUnget = false;
                        }
                    }
                }
            }

            if (!input.EndOfInput() && needToUnget)
            {
                // cout << "in needTO.." << endl;
                input.UngetChar(c);
            }
            // TODO: You can check for REALNUM, BASE08NUM and BASE16NUM here!
        }
        // cout << "flag1" << endl;
        tmp.token_type = NUM;
        tmp.line_no = line_no;
        return tmp;
    }

    else
    {
        if (!input.EndOfInput())
        {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
        tmp.line_no = line_no;
        return tmp;
    }
}

Token LexicalAnalyzer::ScanIdOrKeyword()
{
    char c;
    input.GetChar(c);

    if (isalpha(c))
    {
        tmp.lexeme = "";
        while (!input.EndOfInput() && isalnum(c))
        {
            tmp.lexeme += c;
            input.GetChar(c);
        }
        if (!input.EndOfInput())
        {
            input.UngetChar(c);
        }
        tmp.line_no = line_no;
        if (IsKeyword(tmp.lexeme))
            tmp.token_type = FindKeywordIndex(tmp.lexeme);
        else
            tmp.token_type = ID;
    }
    else
    {
        if (!input.EndOfInput())
        {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}

// you should unget tokens in the reverse order in which they
// are obtained. If you execute
//
//    t1 = lexer.GetToken();
//    t2 = lexer.GetToken();
//    t3 = lexer.GetToken();
//
// in this order, you should execute
//
//    lexer.UngetToken(t3);
//    lexer.UngetToken(t2);
//    lexer.UngetToken(t1);
//
// if you want to unget all three tokens. Note that it does not
// make sense to unget t1 without first ungetting t2 and t3
//
TokenType LexicalAnalyzer::UngetToken(Token tok)
{
    tokens.push_back(tok);
    ;
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken()
{
    char c;

    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if (!tokens.empty())
    {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    SkipSpace();
    tmp.lexeme = "";
    tmp.line_no = line_no;
    input.GetChar(c);
    switch (c)
    {
    case '.':
        tmp.token_type = DOT;
        return tmp;
    case '+':
        tmp.token_type = PLUS;
        return tmp;
    case '-':
        tmp.token_type = MINUS;
        return tmp;
    case '/':
        tmp.token_type = DIV;
        return tmp;
    case '*':
        tmp.token_type = MULT;
        return tmp;
    case '=':
        tmp.token_type = EQUAL;
        return tmp;
    case ':':
        tmp.token_type = COLON;
        return tmp;
    case ',':
        tmp.token_type = COMMA;
        return tmp;
    case ';':
        tmp.token_type = SEMICOLON;
        return tmp;
    case '[':
        tmp.token_type = LBRAC;
        return tmp;
    case ']':
        tmp.token_type = RBRAC;
        return tmp;
    case '(':
        tmp.token_type = LPAREN;
        return tmp;
    case ')':
        tmp.token_type = RPAREN;
        return tmp;
    case '<':
        input.GetChar(c);
        if (c == '=')
        {
            tmp.token_type = LTEQ;
        }
        else if (c == '>')
        {
            tmp.token_type = NOTEQUAL;
        }
        else
        {
            if (!input.EndOfInput())
            {
                input.UngetChar(c);
            }
            tmp.token_type = LESS;
        }
        return tmp;
    case '>':
        input.GetChar(c);
        if (c == '=')
        {
            tmp.token_type = GTEQ;
        }
        else
        {
            if (!input.EndOfInput())
            {
                input.UngetChar(c);
            }
            tmp.token_type = GREATER;
        }
        return tmp;
    default:
        if (isdigit(c))
        {
            input.UngetChar(c);
            return ScanNumber();
        }
        else if (isalpha(c))
        {
            input.UngetChar(c);
            return ScanIdOrKeyword();
        }
        else if (input.EndOfInput())
            tmp.token_type = END_OF_FILE;
        else
            tmp.token_type = ERROR;

        return tmp;
    }
}

int main()
{
    LexicalAnalyzer lexer;
    Token token;

    token = lexer.GetToken();
    token.Print();
    while (token.token_type != END_OF_FILE)
    {
        token = lexer.GetToken();
        token.Print();
    }
}
