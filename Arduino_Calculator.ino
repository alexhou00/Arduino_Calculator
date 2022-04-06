// To do:
// weather station
// complex calculator
// bulls and cows 5 digits maybe wordle (?)
// piano
// 15 puzzle tone

// To do:
// infix to postfix ... done
// postfix calculations ... done
// decimal division (maybe fractions?) ... partly done
// divide by zero error ... done
// bigger number layout ... done
// negative numbers ... done
// brackets ... done
// catch other errors like //
// Orange until released ... done
// factorial ... done
// exponent ... done
// continuous calculations
// more buttons
// overflow preventions

// I wish that I could use python...
#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // Hardware-specific library
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <Wire.h>

//include new libraries
#include <string>
#include <vector>
#include <math.h>
#include <algorithm>

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

// String: Arduino String() object
// string: std::string

long int answer = 0;
string answer_str = "";
char lastchar = ' ';
string laststring = " ";
vector<string> formula;
string cur_key;
string onScreenStr = "";
char operation = ' ';
bool beentouched = false;
bool tooLong = false;
bool equaled = false;

#define row1x 0
#define boxsize 40

//#define r1x 144
#define extraY 48+4*8-1

#define PRINT(name) printSerial(#name, (name))
#define DEBUG true 


int x, y = 0;

struct number{
    long int num;
    long int remainder = 0;
    long int divisor = 0;
    bool undefined = false;
    long int decimal = 0;
    bool isdecimal = false;
};


String button[4][8] = {
    { "-", "-", "-", "7", "8", "9", "/", "C" },
    { "sin", "pi", "n!", "4", "5", "6", "x"},
    { "cos", "e", "^", "1", "2", "3", "-", "<-" },
    { "tan", "|x|", "sqrt", "0", ".", "Ans", "+", "=" }
};

//piano tones
/*int tones[4][4] = {
    { 392, 440, 494, 523 },
    { 440, 494, 587, 659 },
    { 392, 440, 494, 523 },
    { 440, 494, 587, 659 }
};*/
/*int tonepitch[10] = {
    261, 294, 330, 349, 392, 440, 494, 523, 587, 659
};*/
//telephone tones
int tonetel[10] = {
    190, 173, 674, 715, 401, 191, 753, 172, 444, 212
};

// operators priority
int priority(char op) {
    if (op == '+' || op == '-') return 1;
    else if (op == 'x' || op == '/') return 2;
    else if (op == '!') return 3;
    else if (op == '^') return 4;
    else if (op == '(') return 0;
}

long gcd(long a, long b) {
    if (b == 0) return a;
    else return gcd(b, a % b);
}


void draw() {
    tft.fillScreen(ILI9341_BLACK);

    for (int j=0;j<4;j++){
        for (int i=0;i<8;i++) {
            if (i!=7 || j!=1){
                if (i>2 && i<6) tft.drawRoundRect(row1x+i*boxsize, extraY+j*boxsize, boxsize, boxsize, 8, ILI9341_WHITE);
                else tft.drawRoundRect(row1x+i*boxsize, extraY+j*boxsize, boxsize, boxsize, 8, ILI9341_BLUE);
                tft.setCursor(12+i*boxsize-8*(button[j][i].length()>1)+6*(button[j][i].length()==2), extraY+j*boxsize+9+6*(button[j][i].length()>1));
                tft.setTextColor(ILI9341_WHITE);
                tft.setTextSize(3-(button[j][i].length()>1));
                tft.print(button[j][i]);
            }
            else{
                tft.drawRoundRect(row1x+i*boxsize, extraY+j*boxsize, boxsize/2, boxsize, 8, ILI9341_BLUE);
                tft.drawRoundRect(row1x+i*boxsize+boxsize/2, extraY+j*boxsize, boxsize/2, boxsize, 8, ILI9341_BLUE);
                tft.setCursor(5+7*boxsize, extraY+1*boxsize+13);
                tft.setTextColor(ILI9341_WHITE);
                tft.setTextSize(2);
                tft.print("(");
                tft.setCursor(25+7*boxsize, extraY+1*boxsize+13);
                tft.setTextColor(ILI9341_WHITE);
                tft.setTextSize(2);
                tft.print(")");
            }
        }
    }
    for (int yee=0;yee<4;yee++)
        tft.drawRoundRect(row1x + boxsize * 6, extraY+boxsize*yee, boxsize, boxsize, 8, ILI9341_BLUE);
    tft.drawRoundRect(row1x + boxsize * 7, extraY, boxsize, boxsize, 8, ILI9341_RED); //C
    //tft.drawRoundRect(row1x + boxsize * 7, extraY+boxsize, boxsize/2, boxsize, 8, ILI9341_BLUE);
    //tft.drawRoundRect(row1x + boxsize * 7.5, extraY+boxsize, boxsize/2, boxsize, 8, ILI9341_BLUE);
    tft.drawRoundRect(row1x + boxsize * 7, extraY+boxsize*2, boxsize, boxsize, 8, ILI9341_BLUE);
    tft.drawRoundRect(row1x + boxsize * 7, extraY+boxsize*3, boxsize, boxsize, 8, ILI9341_GREEN);
    /*tft.drawRoundRect(row1x, extraY, boxsize, boxsize, 8, ILI9341_WHITE);
    tft.drawRoundRect(row1x, extraY + boxsize, boxsize, boxsize, 8, ILI9341_WHITE);
    tft.drawRoundRect(row1x, extraY + boxsize * 2, boxsize, boxsize, 8, ILI9341_WHITE);
    tft.drawRoundRect(row1x, extraY + boxsize * 3, boxsize, boxsize, 8, ILI9341_RED);

    for (int b = extraY; b <= 192+4*8-1; b += boxsize){
        tft.drawRoundRect  (row1x + boxsize, b, boxsize, boxsize, 8, ILI9341_WHITE);
        tft.drawRoundRect  (row1x + boxsize * 3, b, boxsize, boxsize, 8, ILI9341_BLUE);
    }
    tft.drawRoundRect(row1x + boxsize * 2, extraY, boxsize, boxsize, 8, ILI9341_WHITE);
    tft.drawRoundRect(row1x + boxsize * 2, extraY + boxsize, boxsize, boxsize, 8, ILI9341_WHITE);
    tft.drawRoundRect(row1x + boxsize * 2, extraY + boxsize * 2, boxsize, boxsize, 8, ILI9341_WHITE);
    tft.drawRoundRect(row1x + boxsize * 2, extraY + boxsize * 3, boxsize, boxsize, 8, ILI9341_GREEN);
    */
    /*for (int j = 0; j < 4; j++) {
        //for (int i = 0; i < 4; i++) {
        for (int i = 0; i < 4; i++) {
            tft.setCursor(12 + (boxsize * i), extraY + 9 + (boxsize * j));
            tft.setTextSize(3);
            tft.setTextColor(ILI9341_WHITE);
            tft.println(button[j][i]);
        }
    }*/
    tft.drawRoundRect(0, 0, 320, extraY, 8, ILI9341_ORANGE);
    tft.setCursor(4,12);
}

void drawFormulaScreen(){
    tft.fillRoundRect(0, 0, 320, extraY, 8, ILI9341_BLACK);
    tft.drawRoundRect(0, 0, 320, extraY, 8, ILI9341_ORANGE);
}

void setup(){
    //key1 = "";
    //key2 = "";
    vector<string> formula;
    cur_key = "";
    pinMode(BL_LED, OUTPUT);
    digitalWrite(BL_LED, HIGH);
    Serial.begin(9600);
    if (DEBUG) Serial.println("Calculator Start");
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
        

        int lenFormula = 0;
        for (string i : formula)
            lenFormula += i.length();
        if (lenFormula > 16){
            tooLong = true;
            if (DEBUG) Serial.println("Too long");
        }
        
        TS_Point p = ts.getPoint();     // Read touchscreen
        beentouched = true;

        x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
        y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

        laststring = " ";
        lastchar = ' ';
        laststring = idbutton();
        lastchar = laststring[0];

        //If input is number
        if (lastchar >= '0' && lastchar <= '9' && !tooLong){
            // tone(D1, tonetel[lastchar - '0']); # TONE HERE <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
            // Uncomment above line if you want tone
            cur_key += lastchar;
            tft.setTextSize(3);
            tft.print(lastchar);
            onScreenStr += lastchar;
            if (DEBUG) Serial.print("pressed ");
            if (DEBUG) Serial.println(cur_key.c_str());
            operation = ' ';
        }

        //If input is an operator
        if ((lastchar == '+' || lastchar == '-' || lastchar == '/' || lastchar == 'x' || lastchar =='!' || lastchar =='^') && !tooLong){
            //                                           if (formula is not empty and )
            if ((!formula.size()) || (formula.size() && !(formula.back()=="(" && cur_key == "" && (lastchar == '-' || lastchar == '+'))) ) {
                formula.push_back(cur_key.c_str()); // push the number
                cur_key = "";
            }
            // Handling negative numbers at first
            if ((formula.size()==0 || formula.back()=="(") && (lastchar == '-' || lastchar == '+')) {
                operation = ' ';
                cur_key += lastchar;
                tft.setTextSize(3);
                tft.print(lastchar);
                onScreenStr += lastchar;
            }
            else{
                operation = lastchar;
                tft.setTextSize(3);
                tft.print(operation);
                onScreenStr += operation;
                string operation_str;
                operation_str.push_back(operation);
                formula.push_back(operation_str);
            }
            if (DEBUG) Serial.print("pressed ");
            if (DEBUG) Serial.println(operation);
        }

        if ((lastchar == '(' || lastchar == ')') && !tooLong){
            if (cur_key != "") formula.push_back(cur_key.c_str()); // push the number
            formula.push_back(laststring);
            cur_key = "";
            tft.setTextSize(3);
            tft.print(lastchar);
            onScreenStr += lastchar;
            if (DEBUG) Serial.print("pressed ");
            if (DEBUG) Serial.println(lastchar);
        }

        //If input is equal
        if (lastchar == '='){
            if (formula.size()){
                formula.push_back(cur_key.c_str());
                equaled = true;
                if (DEBUG) Serial.println("pressed Equal");
                answer_str = calc_stk(formula);
                //answer = calc_stk(formula);
                if (DEBUG) Serial.print("answer: ");
                if (DEBUG) Serial.println(answer_str.c_str());
                //if (DEBUG) Serial.println(answer);
                drawFormulaScreen();
                //tft.fillRoundRect(0, 0, 320, extraY, 8, ILI9341_BLACK);
                //tft.drawRoundRect(0, 0, 320, extraY, 8, ILI9341_ORANGE);
                tft.setCursor(4, 12);
                //tft.print('=');
                if (answer_str == "undefined") {
                    tft.setTextColor(ILI9341_RED);
                    tft.setTextSize(3);
                    tft.print("Math ERROR");
                    tft.setTextColor(ILI9341_WHITE);
                }
                else {
                    tft.print(answer_str.c_str());
                    onScreenStr += answer_str;
                    tft.setTextSize(3);
                }
                //tft.print(answer);
                operation = ' ';
                cur_key = "";
                formula.clear();
                //}
            }
        }

        if (lastchar == 'C') {
            formula.clear();
            answer_str = "";
            answer = 0;
            cur_key = "";
            operation = ' ';
            onScreenStr = "";
            draw();
            equaled = false;
            drawFormulaScreen();
            //tft.fillRoundRect(0, 0, 320, extraY, 8, ILI9341_BLACK);
            //tft.drawRoundRect(0, 0, 320, extraY, 8, ILI9341_ORANGE);
            tft.setCursor(4, 12);
            tooLong = false;
        }
        if (laststring == "<-") {
            PRINT(formula);if (DEBUG) Serial.println("\n"); // debug
            if (onScreenStr.length() > 0) {
                onScreenStr = onScreenStr.substr(0, onScreenStr.length() - 1);
            }
            if (cur_key.length() > 0) {
                cur_key = cur_key.substr(0, cur_key.length() - 1);
            }
            else if (formula.back()== "+" || formula.back()== "-" || formula.back()== "x" || formula.back()== "/" || formula.back()== "^" || formula.back()== "!") {
                operation = ' ';
                formula.pop_back();
            }
            else{
                cur_key = formula.back();
                formula.pop_back();
            }
            // equaled = false;
            tft.setCursor(4, 12);
            drawFormulaScreen();
            tft.print(onScreenStr.c_str());
        }

        //wait for release
        while (ts.touched()) {delay(10);};
        beentouched = false;
        noTone(D1);
        }
    }

string idbutton(){
    //Column 3 identification
    if ((x>= boxsize * 2) && (x <= boxsize * 3)){
    //    if (DEBUG) Serial.println("Row 1  ");
    //7
        if (((extraY + boxsize) >= y) && (y >= extraY)) {
            tft.drawRoundRect(row1x + boxsize*2, extraY, boxsize, boxsize, 8, ILI9341_WHITE);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize*2, extraY, boxsize, boxsize, 8, ILI9341_BLUE);
            return "NaN";
        }
        //!
        if (((extraY + (boxsize * 2)) >= y) && (y >= (extraY + boxsize))) {
            tft.drawRoundRect(row1x + boxsize*2, extraY + boxsize, boxsize, boxsize, 8, ILI9341_WHITE);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize*2, extraY + boxsize, boxsize, boxsize, 8, ILI9341_BLUE);
            return "!";
        }
        //^
        if (((extraY + (boxsize * 3)) >= y) && (y >= (extraY + (boxsize * 2)))) {
            tft.drawRoundRect(row1x + boxsize*2, extraY + boxsize * 2, boxsize, boxsize, 8, ILI9341_WHITE);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize*2, extraY + boxsize * 2, boxsize, boxsize, 8, ILI9341_BLUE);
            return "^";
        }
        //sqrt
        if (((extraY + (boxsize * 4)) >= y) && (y >= (extraY + (boxsize * 3)))) {
            tft.drawRoundRect(row1x + boxsize*2, extraY + boxsize * 3, boxsize, boxsize, 8, ILI9341_WHITE);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize*2, extraY + boxsize * 3, boxsize, boxsize, 8, ILI9341_BLUE);
            return "sqrt(";
        }
    }
    //Column 4 identification
    if ((x>= boxsize * 3) && (x <= boxsize * 4)){
    //    if (DEBUG) Serial.println("Row 1  ");
    //7
        if (((extraY + boxsize) >= y) && (y >= extraY)) {
            tft.drawRoundRect(row1x + boxsize*3, extraY, boxsize, boxsize, 8, ILI9341_RED);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize*3, extraY, boxsize, boxsize, 8, ILI9341_WHITE);
            return "7";
        }
        //4
        if (((extraY + (boxsize * 2)) >= y) && (y >= (extraY + boxsize))) {
            tft.drawRoundRect(row1x + boxsize*3, extraY + boxsize, boxsize, boxsize, 8, ILI9341_RED);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize*3, extraY + boxsize, boxsize, boxsize, 8, ILI9341_WHITE);
            return "4";
        }
        //1
        if (((extraY + (boxsize * 3)) >= y) && (y >= (extraY + (boxsize * 2)))) {
            tft.drawRoundRect(row1x + boxsize*3, extraY + boxsize * 2, boxsize, boxsize, 8, ILI9341_RED);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize*3, extraY + boxsize * 2, boxsize, boxsize, 8, ILI9341_WHITE);
            return "1";
        }
        //C
        if (((extraY + (boxsize * 4)) >= y) && (y >= (extraY + (boxsize * 3)))) {
            tft.drawRoundRect(row1x + boxsize*3, extraY + boxsize * 3, boxsize, boxsize, 8, ILI9341_WHITE);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize*3, extraY + boxsize * 3, boxsize, boxsize, 8, ILI9341_RED);
            return "0";
        }
    }

    //Column 5 identification
    if ((x>=boxsize * 4) && (x <= (boxsize * 5))) {
        //    if (DEBUG) Serial.println("Row 2  ");
        //8
        if (((extraY + boxsize) >= y) && (y >= extraY)) {
            tft.drawRoundRect(row1x + boxsize*4, extraY, boxsize, boxsize, 8, ILI9341_RED);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize*4, extraY, boxsize, boxsize, 8, ILI9341_WHITE);
            return "8";
        }
        //5
        if (((extraY + (boxsize * 2)) >= y) && (y >= (extraY + boxsize))) {
            tft.drawRoundRect(row1x + boxsize*4, extraY + boxsize, boxsize, boxsize, 8, ILI9341_RED);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize*4, extraY + boxsize, boxsize, boxsize, 8, ILI9341_WHITE);
            return "5";
        }
        //2
        if (((extraY + (boxsize * 3)) >= y) && (y >= (extraY + (boxsize * 2)))) {
            tft.drawRoundRect(row1x + boxsize*4, extraY + boxsize * 2, boxsize, boxsize, 8, ILI9341_RED);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize*4, extraY + boxsize * 2, boxsize, boxsize, 8, ILI9341_WHITE);
            return "2";
        }
        //0
        if (((extraY + (boxsize * 4)) >= y) && (y >= (extraY + (boxsize * 3)))) {
            tft.drawRoundRect(row1x + boxsize*4, extraY + boxsize * 3, boxsize, boxsize, 8, ILI9341_RED);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize*4, extraY + boxsize * 3, boxsize, boxsize, 8, ILI9341_WHITE);
            return ".";
        }
    }

    //Column 6 identification
    if ((x>=(boxsize * 5)) && (x <= (boxsize * 6))){
        //    if (DEBUG) Serial.println("Row 3  ");
        //9
        if (((extraY + boxsize) >= y) && (y >= extraY)) {
            tft.drawRoundRect(row1x + boxsize * 5, extraY, boxsize, boxsize, 8, ILI9341_RED);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize * 5, extraY, boxsize, boxsize, 8, ILI9341_WHITE);
            return "9";
        }
        //6
        if (((extraY + (boxsize * 2)) >= y) && (y >= (extraY + boxsize))){
            tft.drawRoundRect(row1x + boxsize * 5, extraY + boxsize, boxsize, boxsize, 8, ILI9341_RED);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize * 5, extraY + boxsize, boxsize, boxsize, 8, ILI9341_WHITE);
            return "6";
        }
        //3
        if (((extraY + (boxsize * 3)) >= y) && (y >= (extraY + (boxsize * 2)))){
            tft.drawRoundRect(row1x + boxsize * 5, extraY + boxsize * 2, boxsize, boxsize, 8, ILI9341_RED);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize * 5, extraY + boxsize * 2, boxsize, boxsize, 8, ILI9341_WHITE);
            return "3";
        }
        //=
        if (((extraY + (boxsize * 4)) >= y) && (y >= (extraY + (boxsize * 3)))){
            tft.drawRoundRect(row1x + boxsize * 5, extraY + boxsize * 3, boxsize, boxsize, 8, ILI9341_BLUE);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize * 5, extraY + boxsize * 3, boxsize, boxsize, 8, ILI9341_GREEN);
            return "Ans";
        }
    }

    //Column 7 identification
    if ((x>=(boxsize * 6)) && (x <= (boxsize * 7))) {
        //    if (DEBUG) Serial.println("Row 4  ");
        //+
        if (((extraY + boxsize) >= y) && (y >= extraY)){
            tft.drawRoundRect(row1x + boxsize * 6, extraY, boxsize, boxsize, 8, ILI9341_GREEN);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize * 6, extraY, boxsize, boxsize, 8, ILI9341_BLUE);
            return "/";
        }
        //-
        if (((extraY + (boxsize * 2)) >= y) && (y >= (extraY + boxsize))) {
            tft.drawRoundRect(row1x + boxsize * 6, extraY + boxsize, boxsize, boxsize, 8, ILI9341_GREEN);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize * 6, extraY + boxsize, boxsize, boxsize, 8, ILI9341_BLUE);
            return "x";
        }
        //*
        if (((extraY + (boxsize * 3)) >= y) && (y >= (extraY + (boxsize * 2)))){
            tft.drawRoundRect(row1x + boxsize * 6, extraY + boxsize * 2, boxsize, boxsize, 8, ILI9341_GREEN);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize * 6, extraY + boxsize * 2, boxsize, boxsize, 8, ILI9341_BLUE);
            return "-";
        }
        // /
        if (((extraY + (boxsize * 4)) >= y) && (y >= (extraY + (boxsize * 3)))) {
            tft.drawRoundRect(row1x + boxsize * 6, extraY + boxsize * 3, boxsize, boxsize, 8, ILI9341_GREEN);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize * 6, extraY + boxsize * 3, boxsize, boxsize, 8, ILI9341_BLUE);
            return "+";
        }
    }
    
    //Column 8 identification
    if ((x>=(boxsize * 7)) && (x <= (boxsize * 8))) {
        //    if (DEBUG) Serial.println("Row 4  ");
        //+
        if (((extraY + boxsize) >= y) && (y >= extraY)){
            tft.drawRoundRect(row1x + boxsize * 7, extraY, boxsize, boxsize, 8, ILI9341_GREEN);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize * 7, extraY, boxsize, boxsize, 8, ILI9341_BLUE);
            return "C";
        }
        //-
        if (((extraY + (boxsize * 2)) >= y) && (y >= (extraY + boxsize))) {
            if (x<(boxsize * 7.5)) {
                tft.drawRoundRect(row1x + boxsize * 7, extraY + boxsize, boxsize/2, boxsize, 8, ILI9341_GREEN);
                while (ts.touched()) {delay(10);};
                tft.drawRoundRect(row1x + boxsize * 7, extraY + boxsize, boxsize/2, boxsize, 8, ILI9341_BLUE);
                return "(";
            }
            else{
                tft.drawRoundRect(row1x + boxsize * 7.5, extraY + boxsize, boxsize/2, boxsize, 8, ILI9341_GREEN);
                while (ts.touched()) {delay(10);};
                tft.drawRoundRect(row1x + boxsize * 7.5, extraY + boxsize, boxsize/2, boxsize, 8, ILI9341_BLUE);
                return ")";
            }
        }
        //*
        if (((extraY + (boxsize * 3)) >= y) && (y >= (extraY + (boxsize * 2)))){
            tft.drawRoundRect(row1x + boxsize * 7, extraY + boxsize * 2, boxsize, boxsize, 8, ILI9341_GREEN);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize * 7, extraY + boxsize * 2, boxsize, boxsize, 8, ILI9341_BLUE);
            return "<-";
        }
        // /
        if (((extraY + (boxsize * 4)) >= y) && (y >= (extraY + (boxsize * 3)))) {
            tft.drawRoundRect(row1x + boxsize * 7, extraY + boxsize * 3, boxsize, boxsize, 8, ILI9341_GREEN);
            //delay(100);
            while (ts.touched()) {delay(10);};
            tft.drawRoundRect(row1x + boxsize * 7, extraY + boxsize * 3, boxsize, boxsize, 8, ILI9341_BLUE);
            return "=";
        }
    }
}

long int factorial(long int n) {
    /*if (n == 0) {
        return 1;
    }
    else {
        return n * factorial(n - 1);
    }*/
    return (n == 1 || n == 0) ? 1 : n * factorial(n - 1);
}

number calc(number num1, number num2, char op){
    number tmp;
    switch (op) {
        case '+':
            tmp.num = num1.num + num2.num;
            if (num1.remainder || num2.remainder){
                tmp.remainder = num1.remainder*num2.divisor + num2.remainder*num1.divisor;
                tmp.divisor = num1.divisor*num2.divisor;
            }
            return tmp;
        case '-':
            tmp.num = num1.num - num2.num;
            if (num1.remainder || num2.remainder){
                tmp.remainder = num1.remainder*num2.divisor - num2.remainder*num1.divisor;
                tmp.divisor = num1.divisor*num2.divisor;
            }
            return tmp;
        case 'x':
            // tmp.num = num1.num * num2.num;
            if (num1.remainder || num2.remainder){
                tmp.num = num1.num * num2.num;
                tmp.remainder = num1.num*num1.divisor*num2.remainder+num2.num*num2.divisor*num1.remainder+num1.remainder*num2.remainder;
                tmp.divisor = num1.divisor*num2.divisor;
            }
            else{
                tmp.num = num1.num * num2.num;
            }
            return tmp;
        case '/':
            if (num2.num == 0){
                tmp.undefined = true;
                tmp.num = 0;
                tmp.remainder = 0;
                tmp.divisor = 0;
            }
            else {
                tmp.num = num1.num / num2.num;
                tmp.remainder = num1.num % num2.num;
                tmp.divisor = num2.num;
            }
            return tmp;
        case '_':
            tmp.num = num2.num * -1;
            return tmp;
        case '!':
            if (DEBUG) Serial.println("Factorial");
            tmp.num = factorial(num2.num);
            return tmp;
        case '^':
            if (DEBUG) Serial.println("Exponent");
            tmp.num = pow(num1.num, num2.num);
            return tmp;
    }
}


void printSerial(char *name, auto &iterable){
    if (DEBUG) Serial.print(name);
    if (DEBUG) Serial.print(": ");
    for (auto i : iterable){
        if (DEBUG) Serial.print(i.c_str());
        if (DEBUG) Serial.print(" ");
    }
    if (DEBUG) Serial.print("  size: ");
    if (DEBUG) Serial.print(iterable.size());
    if (DEBUG) Serial.println();
}

number myAtoi(const char* str) {
    number num;
    int res = 0;
    int i = 0;
    for (; !(str[i]=='\0'||str[i]=='+'); ++i)
        res = res * 10 + str[i] - '0';
    if (str[i] == '\0') goto finishAtoi;
    num.num = res;
    res = 0;
    for (i=i+1;!(str[i]=='\0'||str[i]=='/'); ++i)
        res = res * 10 + str[i] - '0';
    num.remainder = res;
    res = 0;
    for (i=i+1;!(str[i]=='\0'); ++i)
        res = res * 10 + str[i] - '0';
    num.divisor = res;
    finishAtoi:
    return num;
}

string calc_stk(vector<string> formula){
    vector<string> stk; // actually stack
    vector<string> postfix;
    string postfix_str = "";
    char operators[7] = {'+', '-', 'x', '/', '_', '!', '^'};
    /*
    *   infix to Postfix conversion
    *                       stk
    *   postfix:_______    |   |   formula:____
    *                 ↑    |   |    │
    *                 |    |   |  <─┘
    *                 └──  └───┘
    */
    if (DEBUG) Serial.println("Ready to Calculate");
    PRINT(formula);PRINT(postfix);PRINT(stk);if (DEBUG) Serial.println("\n");
    for (string s : formula){
        if (DEBUG) Serial.print("Processing ");
        if (DEBUG) Serial.println(s.c_str());
        if (s[0] == '('){
            /*string delimiter = "+";
            string token = s.substr(0, s.find(delimiter));
            s.erase(0, s.find(delimiter) + delimiter.length());*/
            stk.push_back(s);
            
            PRINT(formula);PRINT(postfix);PRINT(stk);if (DEBUG) Serial.println("\n");
        }
        else if (s[0] == ')'){
            while (stk.size() && stk.back()[0]!='('){
                postfix.push_back(stk.back());
                stk.pop_back();
            }
            if (stk.size()){
                if (stk.back()[0]=='(') stk.pop_back();
            }
        }
        else if (s.length() == 1 && !isdigit(s[0])){
            for (auto op : operators){
                if (s[0] == op){
                    while (stk.size() && priority(op)<=priority(stk.back()[0])){
                        postfix.push_back(stk.back());
                        PRINT(formula);PRINT(postfix);PRINT(stk);if (DEBUG) Serial.println("\n");
                        //if (DEBUG) Serial.print("postfix push back ");
                        //if (DEBUG) Serial.println(stk.back().c_str());
                        stk.pop_back();
                        PRINT(formula);PRINT(postfix);PRINT(stk);if (DEBUG) Serial.println("\n");
                    }
                    stk.push_back(s);
                    PRINT(formula);PRINT(postfix);PRINT(stk);if (DEBUG) Serial.println("\n");
                }
            }
        }
        else if (s != ""){
            postfix.push_back(s);
            PRINT(formula);PRINT(postfix);PRINT(stk);if (DEBUG) Serial.println("\n");
        }
    }
    while (stk.size()) {
        postfix.push_back(stk.back());
        stk.pop_back();
        PRINT(formula);PRINT(postfix);PRINT(stk);if (DEBUG) Serial.println("\n");
    }
    /*
    * Calculate postfix
    *
    */
    number num1, num2;
    number result;
    result.num = 0;
    string result_str = "";
    for (auto s : postfix){
        PRINT(formula);PRINT(postfix);PRINT(stk);if (DEBUG) Serial.println("\n");
        if (isdigit(s[0]) || (s.length()>1 && s[0] == '-' && isdigit(s[1]))){
            result.num = atoi(s.c_str());
            //if (DEBUG) Serial.print("result is ");
            //if (DEBUG) Serial.print(result);println(result.num);
        }
        else{
            char c = stk.back()[0];
            if (DEBUG) Serial.println("stk fetched");
            num2 = myAtoi(stk.back().c_str());
            stk.pop_back();
            if (DEBUG) Serial.println("stk fetched and popped");
            if (s[0] != '!'){ // single operand operator
                num1 = myAtoi(stk.back().c_str());
                stk.pop_back();
                if (DEBUG) Serial.println("stk fetched and popped");
            }
            else num1.num = 0;
            if (DEBUG) Serial.println("calc");
            // number num1
            result = calc(num1, num2, s[0]);
            if (DEBUG) Serial.print("result in int is ");
            if (DEBUG) Serial.println(result.num);
            //if (result.remainder != 0) if (DEBUG) Serial.println(result.remainder);
        }
        if (result.remainder){
            long gcd_ = gcd(result.remainder, result.divisor);
            if (gcd_ != 1){
                result.remainder /= gcd_;
                result.divisor /= gcd_;
            }
            if (result.remainder*result.divisor < 0){
                if (result.remainder < 0)
                    result.remainder *= -1;
                else
                    result.divisor *= -1;
            }
            if (result.num == 0)
                result_str = to_string(result.remainder) + "/" + to_string(result.divisor);
            else
                result_str = to_string(result.num) + " " + to_string(result.remainder) + "/" + to_string(result.divisor);
        }
        else{
            result_str = to_string(result.num);
        }
        // result_str = to_string(result.num)+" "+to_string(result.remainder)+"/"+to_string(result.divisor);
        stk.push_back(to_string(result.num) + "+" + to_string(result.remainder) + "/" + to_string(result.divisor));
    }
    if (result.undefined) return "undefined";
    return result_str;
}
