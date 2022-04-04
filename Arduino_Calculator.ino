// To do:
// weather station
// complex calculator
// bulls and cows 5 digits maybe wordle (?)

// To do:
// infix to postfix ... done
// postfix calculations ... done
// decimal division
// divide by zero error 
// catch other errors
// Orange until released
// more buttons

// I wish that I could use python...
#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // Hardware-specific library
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <Wire.h>

//#include <stack>
#include <string>
#include <vector>
#include <math.h>
//#include <ArduinoSTL.h>

using namespace std;

#define TOUCH_CS_PIN D3
#define TOUCH_IRQ_PIN D2

#define TS_MINX 330
#define TS_MINY 213
#define TS_MAXX 3963
#define TS_MAXY 3890

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
XPT2046_Touchscreen ts(TOUCH_CS_PIN);

// The display also uses hardware SPI(D5,D6,D7), SD3, D4, D8
static uint8_t SD3 = 10;
#define TFT_CS SD3
#define TFT_DC D4
#define BL_LED D8

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

long int answer = 0;
string answer2 = "";
char lastchar = ' ';
String key1;
String key2;
vector<string> formula;
String cur_key;
int key1i = 0;
int key2i = 0;
char operation = ' ';
bool beentouched = false;
bool toLong = false;
bool equaled = false;

#define row1x 0
#define boxsize 48

#define r1x 144
#define extray 48

int x, y = 0;


char button[4][4] = {
    { '7', '8', '9', '/' },
    { '4', '5', '6', 'x' },
    { '1', '2', '3', '-' },
    { 'C', '0', '=', '+' }
};
int tones[4][4] = {
    { 392, 440, 494, 523 },
    { 440, 494, 587, 659 },
    { 392, 440, 494, 523 },
    { 440, 494, 587, 659 }
};
int tonsse[10] = {
    261, 294, 330, 349, 392, 440, 494, 523, 587, 659
};
int tonetel[10] = {
    190, 173, 674, 715, 401, 191, 753, 172, 444, 212
};
// # 

int priority(char op) {
    if (op == '+' || op == '-') return 1;
    else if (op == 'x' || op == '/') return 2;
    else if (op == '(') return 0;
}

void draw() {
    tft.fillScreen(ILI9341_BLACK);

    tft.drawRoundRect(row1x, extray, boxsize, boxsize, 8, ILI9341_WHITE);
    tft.drawRoundRect(row1x, extray + boxsize, boxsize, boxsize, 8, ILI9341_WHITE);
    tft.drawRoundRect(row1x, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_WHITE);
    tft.drawRoundRect(row1x, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_RED);

    for (int b = extray; b <= 192; b += boxsize){
        tft.drawRoundRect  (row1x + boxsize, b, boxsize, boxsize, 8, ILI9341_WHITE);
        tft.drawRoundRect  (row1x + boxsize * 3, b, boxsize, boxsize, 8, ILI9341_BLUE);
    }
    tft.drawRoundRect(row1x + boxsize * 2, extray, boxsize, boxsize, 8, ILI9341_WHITE);
    tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize, boxsize, boxsize, 8, ILI9341_WHITE);
    tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_WHITE);
    tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_GREEN);

    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            tft.setCursor(16 + (boxsize * i), extray + 12 + (boxsize * j));
            tft.setTextSize(3);
            tft.setTextColor(ILI9341_WHITE);
            tft.println(button[j][i]);
        }
    }
    tft.drawRoundRect(0, 0, 320, 48, 8, ILI9341_ORANGE);
    tft.setCursor(4,12);
}

void setup(){
    key1 = "";
    key2 = "";
    vector<string> formula;
    cur_key = "";
    pinMode(BL_LED, OUTPUT);
    digitalWrite(BL_LED, HIGH);
    Serial.begin(9600);
    Serial.println("Calculator");
    ts.begin();
    tft.begin();
    tft.setRotation(3);
    draw();
    tft.setCursor(4, 12);
}

void loop(){
    if (ts.touched() && !beentouched){
        if(equaled){
            equaled = false;
            draw();
        }
        
        /*if(key1.length()+key2.length()+int(operation != ' ') > 16){
            toLong = true;
        }*/
        int lenFormula = 0;
        for (auto i : formula)
            lenFormula += i.length();
        if (lenFormula > 16){
            toLong = true;
        }
        
        TS_Point p = ts.getPoint();     // Read touchscreen
        beentouched = true;

        x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
        y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

        lastchar = ' ';
        lastchar = idbutton();

        //If input is number
        if (lastchar >= '0' && lastchar <= '9' && !toLong){
            // tone(D1, tonetel[lastchar - '0']); # TONE HERE <<<<<<<<<<<<<<<<<<<<<<<<<
            // Uncomment above line if you want tone
            //If input is digitr & operation is not defined
            cur_key += lastchar;
            tft.print(lastchar);
            Serial.println(cur_key);
            operation = ' ';
            /*if (operation == ' '){
                key1 += lastchar;
                tft.print(lastchar);
                Serial.println(key1);
            }
            //If input is digit & operation is defined
            else{
                key2 += lastchar;
                tft.print(lastchar);
                Serial.println(key2);
            }*/
        }

        //If input is an operator
        /*if ((lastchar == '+' || lastchar == '-' || lastchar == '/' || lastchar == 'x') && key2 == "" && key1 != "" && !toLong){
            if ( operation != ' ') {
                operation = lastchar;
                tft.fillRoundRect(0, 0, 320, 48, 8, ILI9341_BLACK);
                tft.drawRoundRect(0, 0, 320, 48, 8, ILI9341_ORANGE);
                tft.setCursor(4, 12);
                tft.print(key1);
                tft.print(operation);
            }

            //If input is an operator and operation is not defined
            else {
                operation = lastchar;
                tft.print(operation);

            }
        }*/
        if ((lastchar == '+' || lastchar == '-' || lastchar == '/' || lastchar == 'x') && !toLong){
            formula.push_back(cur_key.c_str());
            cur_key = "";
            operation = lastchar;
            tft.print(operation);
            string operation_str;
            operation_str.push_back(operation);
            formula.push_back(operation_str);
        }

        //If input is equal
        if (lastchar == '='){
            //If neither key1 or key2 are empty
            //if (key1 != "" && key2 != ""){
            if (formula.size()){
                formula.push_back(cur_key.c_str());
                equaled = true;
                Serial.println("Calculate");
                //Check devide by zero error
                /*if (key2.toInt() == 0 && operation == '/'){
                    tft.setCursor(4, 12);
                    tft.fillRoundRect(0, 0, 320, 48, 8, ILI9341_BLACK);
                    tft.drawRoundRect(0, 0, 320, 48, 8, ILI9341_ORANGE);
                    tft.print("=");
                    tft.setTextColor(ILI9341_RED);
                    tft.print("ERROR");
                    tft.setTextColor(ILI9341_WHITE);
                    key1 = "";
                    key2 = "";
                    formula = "";
                    cur_key = "";
                    operation = ' ';
                }*/
                //else {
                    /*key1i = 0;
                    key2i = 0;
                    key1i = key1.toInt();
                    key2i = key2.toInt();*/
                    //answer = calc(key1i, key2i, operation);
                //answer2 = calc_stk(formula);
                answer = calc_stk(formula);
                Serial.print("answer: ");
                //Serial.println(answer2.c_str());
                Serial.println(answer);
                tft.fillRoundRect(0, 0, 320, 48, 8, ILI9341_BLACK);
                tft.drawRoundRect(0, 0, 320, 48, 8, ILI9341_ORANGE);
                tft.setCursor(4, 12);
                    //tft.print('=');
                //tft.print(answer2.c_str());
                tft.print(answer);
                    /*key1i = 0;
                    key2i = 0;*/
                operation = ' ';
                    /*key1 = "";
                    key2 = "";*/
                cur_key = "";
                formula.clear();
                //}
            }
        }

        if (lastchar == 'C') {
            /*key1 = "";
            key2 = "";*/
            formula.clear();
            answer2 = "";
            answer = 0;

            /*key1i = 0;
            key2i = 0;*/

            operation = ' ';
            draw();
            equaled = false;
            tft.fillRoundRect(0, 0, 320, 48, 8, ILI9341_BLACK);
            tft.drawRoundRect(0, 0, 320, 48, 8, ILI9341_ORANGE);
            tft.setCursor(4, 12);
            toLong = false;
        }

        //wait for release
        while (ts.touched()) {delay(10);};
        beentouched = false;
        noTone(D1);
        }
    }

char idbutton(){
  //Column 1 identification
  if ((x>=0) && (x <= boxsize))
  {
    //    Serial.println("Row 1  ");
    //7
    if (((extray + boxsize) >= y) && (y >= extray)) {
        tft.drawRoundRect(row1x, extray, boxsize, boxsize, 8, ILI9341_RED);
        delay(100);
        tft.drawRoundRect(row1x, extray, boxsize, boxsize, 8, ILI9341_WHITE);
        return '7';
    }
    //4
    if (((extray + (boxsize * 2)) >= y) && (y >= (extray + boxsize))) {
        tft.drawRoundRect(row1x, extray + boxsize, boxsize, boxsize, 8, ILI9341_RED);
        delay(100);
        tft.drawRoundRect(row1x, extray + boxsize, boxsize, boxsize, 8, ILI9341_WHITE);
        return '4';
    }
    //1
    if (((extray + (boxsize * 3)) >= y) && (y >= (extray + (boxsize * 2)))) {
        tft.drawRoundRect(row1x, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_RED);
        delay(100);
        tft.drawRoundRect(row1x, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_WHITE);
        return '1';
    }
    //C
    if (((extray + (boxsize * 4)) >= y) && (y >= (extray + (boxsize * 3)))) {
        tft.drawRoundRect(row1x, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_WHITE);
        delay(100);
        tft.drawRoundRect(row1x, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_RED);
        return 'C';
    }

  }

  //Column 2 identification
  if ((x>=boxsize) && (x <= (boxsize * 2))) {
    //    Serial.println("Row 2  ");
    //8
    if (((extray + boxsize) >= y) && (y >= extray)) {
        tft.drawRoundRect(row1x + boxsize, extray, boxsize, boxsize, 8, ILI9341_RED);
        delay(100);
        tft.drawRoundRect(row1x + boxsize, extray, boxsize, boxsize, 8, ILI9341_WHITE);
        return '8';
    }
    //5
    if (((extray + (boxsize * 2)) >= y) && (y >= (extray + boxsize))) {
        tft.drawRoundRect(row1x + boxsize, extray + boxsize, boxsize, boxsize, 8, ILI9341_RED);
        delay(100);
        tft.drawRoundRect(row1x + boxsize, extray + boxsize, boxsize, boxsize, 8, ILI9341_WHITE);
        return '5';
    }
    //2
    if (((extray + (boxsize * 3)) >= y) && (y >= (extray + (boxsize * 2)))) {
        tft.drawRoundRect(row1x + boxsize, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_RED);
        delay(100);
        tft.drawRoundRect(row1x + boxsize, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_WHITE);
        return '2';
    }
    //0
    if (((extray + (boxsize * 4)) >= y) && (y >= (extray + (boxsize * 3)))) {
        tft.drawRoundRect(row1x + boxsize, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_RED);
        delay(100);
        tft.drawRoundRect(row1x + boxsize, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_WHITE);
        return '0';
    }
  }

  //Column 3 identification
  if ((x>=(boxsize * 2)) && (x <= (boxsize * 3))){
    //    Serial.println("Row 3  ");
    //9
      if (((extray + boxsize) >= y) && (y >= extray)) {
        tft.drawRoundRect(row1x + boxsize * 2, extray, boxsize, boxsize, 8, ILI9341_RED);
        delay(100);
        tft.drawRoundRect(row1x + boxsize * 2, extray, boxsize, boxsize, 8, ILI9341_WHITE);
        return '9';
      }
    //6
    if (((extray + (boxsize * 2)) >= y) && (y >= (extray + boxsize))){
        tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize, boxsize, boxsize, 8, ILI9341_RED);
        delay(100);
        tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize, boxsize, boxsize, 8, ILI9341_WHITE);
        return '6';
    }
    //3
    if (((extray + (boxsize * 3)) >= y) && (y >= (extray + (boxsize * 2)))){
        tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_RED);
        delay(100);
        tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_WHITE);
        return '3';
    }
    //=
    if (((extray + (boxsize * 4)) >= y) && (y >= (extray + (boxsize * 3)))){
        tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_BLUE);
        delay(100);
        tft.drawRoundRect(row1x + boxsize * 2, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_GREEN);
        return '=';
    }
  }

  //Column 4 identification
  if ((x>=(boxsize * 3)) && (x <= (boxsize * 4))) {
        //    Serial.println("Row 4  ");
        //+
        if (((extray + boxsize) >= y) && (y >= extray)){
            tft.drawRoundRect(row1x + boxsize * 3, extray, boxsize, boxsize, 8, ILI9341_GREEN);
            delay(100);
            tft.drawRoundRect(row1x + boxsize * 3, extray, boxsize, boxsize, 8, ILI9341_BLUE);
            return '/';
        }
        //-
        if (((extray + (boxsize * 2)) >= y) && (y >= (extray + boxsize))) {
            tft.drawRoundRect(row1x + boxsize * 3, extray + boxsize, boxsize, boxsize, 8, ILI9341_GREEN);
            delay(100);
            tft.drawRoundRect(row1x + boxsize * 3, extray + boxsize, boxsize, boxsize, 8, ILI9341_BLUE);
            return 'x';
        }
        //*
        if (((extray + (boxsize * 3)) >= y) && (y >= (extray + (boxsize * 2)))){
            tft.drawRoundRect(row1x + boxsize * 3, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_GREEN);
            delay(100);
            tft.drawRoundRect(row1x + boxsize * 3, extray + boxsize * 2, boxsize, boxsize, 8, ILI9341_BLUE);
            return '-';
        }
        // /
        if (((extray + (boxsize * 4)) >= y) && (y >= (extray + (boxsize * 3)))) {
            tft.drawRoundRect(row1x + boxsize * 3, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_GREEN);
            delay(100);
            tft.drawRoundRect(row1x + boxsize * 3, extray + boxsize * 3, boxsize, boxsize, 8, ILI9341_BLUE);
            return '+';
        }
    }
}

long int calc(long int num1, long int num2, char op){
    switch (op) {
        case '+':
        return num1 + num2;
        case '-':
        return num1 - num2;
        case 'x':
        return num1 * num2;
        case '/':
        return num1 / num2;
    }
}

long int calc_stk(vector<string> formula){
    vector<string> stk; // actually stack
    vector<string> postfix;
    string postfix_str = "";
    char operators[5] = {'+', '-', 'x', '/'};
    /*
    *   Prefix to Postfix conversion
    *                       stk
    *   postfix:_______    |   |   formula:____
    *                 ^    |   |    |
    *                 |    |   |  <--
    *                  --  -----
    */
    Serial.println("Ready to Calculate");
    Serial.println("Formula is");
    for (string s : formula){
        Serial.println(s.c_str());
    }
    for (string s : formula){
        Serial.print("Processing ");
        Serial.println(s.c_str());
        if (s[0] == '('){
            stk.push_back(s);
        }
        else if (s[0] == ')'){
            while (stk.size() && stk.back()[0]!='('){
                postfix.push_back(stk.back());
                stk.pop_back();
            }
            if (stk.size()){
                if (stk.back()[0]=='(') stk.back();
            }
        }
        else if (s.length() == 1 && !isdigit(s[0])){
            for (auto op : operators){
                if (s[0] == op){
                    while (stk.size() && priority(op)<=priority(stk.back()[0])){
                        postfix.push_back(stk.back());
                        Serial.print("postfix push back ");
                        Serial.println(stk.back().c_str());
                        stk.pop_back();
                    }
                    stk.push_back(s);
                    Serial.print("stk push ");
                    Serial.println(s.c_str());
                }
            }
        }
        else{
            postfix.push_back(s);
            Serial.print("postfix push back ");
            Serial.println(s.c_str());
        }
        
        /*if (s.length() == 1){
            for (auto op : operators){
                if (s[0] == op){
                    stk.push(s);
                }
                else{
                    stk.push(s);
                }
                break;
            }
        }
        else{
            stk.push(s);
        }*/
    }
    while (stk.size()) {
        postfix.push_back(stk.back());
        stk.pop_back();
    }
    Serial.println("postfix now is ");
    for (auto s : postfix){
        postfix_str += s;
        Serial.println(postfix_str.c_str());
    }
    /*
    * Calculate postfix
    *
    */
    long int num1, num2;
    long int result = 0;
    for (auto s : postfix){
        if (isdigit(s[0])){
            result = atoi(s.c_str());
            Serial.print("result is ");
            Serial.println(result);
        }
        else{
            num2 = atoi(stk.back().c_str());
            stk.pop_back();
            num1 = atoi(stk.back().c_str());
            stk.pop_back();
            result = calc(num1, num2, s[0]);
            Serial.print("result is ");
            Serial.println(result);
        }
        stk.push_back(to_string(result));
    }
    return result;
}
