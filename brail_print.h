/*************************************************/
// Name		:	brail_print.h
// Author	:	An Jiyoon (Ziyoon)
/*************************************************/

#ifndef BRAIL_PRINT_H
# define BRAIL_PRINT_H

# define BRAIL_1 21
# define BRAIL_2 22
# define BRAIL_3 23
# define BRAIL_4 27
# define BRAIL_5 28
# define BRAIL_6 29
# define NEXT    24
# define SOUND   25

// sudo apt-get install wiringpi
# include <wiringPi.h>
# include <iostream>
# include <fstream>
# include <locale>
# include <codecvt>
# include <string>
# include <cstring>
# include <map>
# include <bitset>

using namespace std;

/*************** PUBLIC  VARIABLES ***************/
// korean character
struct Hangul{
    int number = -1;
    char16_t front;
    char16_t mid;
    char16_t end;
    char16_t end_twice;
}; 

// binary braille
struct Braille{
    unsigned int front_1 = 0;
    unsigned int front_2 = 0;
    unsigned int mid_1 = 0;
    unsigned int mid_2 = 0;
    unsigned int end = 0;
}; 

// Solienoid
struct Solenoid{
    bool bit_1 = 0;
    bool bit_2 = 0;
    bool bit_3 = 0;
    bool bit_4 = 0;
    bool bit_5 = 0;
    bool bit_6 = 0;
};

Solenoid sol;

Braille braille[20];
int name_len;
string prev_line;

// 약품명 변수
u16string p_medicine_name[15];

// 한글 초성, 중성, 종성 리스트
u16string front_list = u"ㄱㄲㄴㄷㄸㄹㅁㅂㅃㅅㅆㅇㅈㅉㅊㅋㅌㅍㅎ";
u16string mid_list   = u"ㅏㅐㅑㅒㅓㅔㅕㅖㅗㅘㅙㅚㅛㅜㅝㅞㅟㅠㅡㅢㅣ";
u16string end_list   = u"ㄱㄲㄳㄴㄵㄶㄷㄹㄺㄻㄼㄽㄾㄿㅀㅁㅂㅄㅅㅆㅇㅈㅊㅋㅌㅍㅎ";
u16string etc_list   = u".,+-=()";

// 1 + 6자 : 맨 앞자리의 1은 쌍자음임을 나타냄
// 쌍자음은 000_001을 앞에 추가로 표기 해야함
map<char16_t, unsigned int> front_letters = {
    {u'ㄱ', 0b0000100},
    {u'ㄲ', 0b1000100}, // 쌍자음
    {u'ㄴ', 0b0100100},
    {u'ㄷ', 0b0010100},
    {u'ㄸ', 0b1010100}, // 쌍자음
    {u'ㄹ', 0b0000010},
    {u'ㅁ', 0b0100010},
    {u'ㅂ', 0b0000110},
    {u'ㅃ', 0b1000110}, // 쌍자음
    {u'ㅅ', 0b0000001},
    {u'ㅆ', 0b1000001}, // 쌍자음
    {u'ㅇ', 0},			// 예외
    {u'ㅈ', 0b0000101},
    {u'ㅉ', 0b1000101}, // 쌍자음
    {u'ㅊ', 0b0000011},
    {u'ㅋ', 0b0110100},
    {u'ㅌ', 0b0110010},
    {u'ㅍ', 0b0100110},
    {u'ㅎ', 0b0010110}
};

// 1 + 6자 : 맨 앞자리의 1은 두개의 점자가 필요함을 표기
// 뒤에 111_010 추가로 표기 해야함
map<char16_t, unsigned int> mid_letters = {
    {u'ㅏ', 0b0110001},
    {u'ㅐ', 0b0111010},
    {u'ㅑ', 0b0001110},
    {u'ㅒ', 0b1001110},  // + (0b111010)
    {u'ㅓ', 0b0011100},
    {u'ㅔ', 0b0101110},
    {u'ㅕ', 0b0100011},
    {u'ㅖ', 0b0001100},
    {u'ㅗ', 0b0101001},
    {u'ㅘ', 0b0111001},
    {u'ㅙ', 0b1111001},  // + (0b111010)
    {u'ㅚ', 0b0101111},
    {u'ㅛ', 0b0001101},
    {u'ㅜ', 0b0101001},
    {u'ㅝ', 0b0111100},
    {u'ㅞ', 0b1111100},  // + (0b111010)
    {u'ㅟ', 0b1101100},  // + (0b111010)
    {u'ㅠ', 0b0100101},
    {u'ㅡ', 0b0010101},
    {u'ㅢ', 0b0010111},
    {u'ㅣ', 0b0101010}
};

// 1 + 6자 : 맨 앞자리는 더미
map<char16_t, unsigned int> end_letters = {
    {u'ㄱ', 0b0100000},
    {u'ㄴ', 0b0010010},
    {u'ㄷ', 0b0001010},
    {u'ㄹ', 0b0010000},
    {u'ㅁ', 0b0010001},
    {u'ㅂ', 0b0110000},
    {u'ㅅ', 0b0001000},
    {u'ㅇ', 0b0011011},
    {u'ㅈ', 0b0101000},
    {u'ㅊ', 0b0011000},
    {u'ㅋ', 0b0011010},
    {u'ㅌ', 0b0011001},
    {u'ㅍ', 0b0010011},
    {u'ㅎ', 0b0001011},
    {u'ㅆ', 0b0001100},	// 예외

    // 발음 대로 구현
    {u'ㄲ', 0b0100000},
    {u'ㄳ', 0b0100000},
    {u'ㄵ', 0b0010010},
    {u'ㄶ', 0b0010010},
    {u'ㄺ', 0b0100000},
    {u'ㄻ', 0b0010001},
    {u'ㄼ', 0b0110000},
    {u'ㄽ', 0b0010000},
    {u'ㄾ', 0b0011001},
    {u'ㄿ', 0b0110000},
    {u'ㅀ', 0b0010000},
    {u'ㅄ', 0b0110000}
};

// number start 이후로 숫자가 나옴
unsigned int number_start = 0b0001111;
map<char, unsigned int> numbers = {
    {'1', 0b0100000},
    {'2', 0b0110000},
    {'3', 0b0100100},
    {'4', 0b0100110},
    {'5', 0b0100010},
    {'6', 0b0110100},
    {'7', 0b0110110},
    {'8', 0b0110010},
    {'9', 0b0010100},
    {'0', 0b0010110} 
};

// 여기서부터 사용 하지 않음
// 추후 기능 업데이트를 위해 넣어둔 코드
map<char16_t, unsigned int> contraction_letters = {
    {u'가', 0b0110101},
    {u'나', 0b0100100},
    {u'다', 0b0010100},
    {u'마', 0b0100010},
    {u'바', 0b0000110},
    {u'사', 0b0111000},
    {u'자', 0b0000101},
    {u'카', 0b0110100},
    {u'타', 0b0110010},
    {u'파', 0b0100110},
    {u'하', 0b0010110},
    // 이후로는 구현 필요가 없으므로 구현 하지 않음
    {u'것', 0b0},
    {u'억', 0b0},
    {u'언', 0b0},
    {u'얼', 0b0},
    {u'연', 0b0},
    {u'열', 0b0},
    {u'영', 0b0},
    {u'옥', 0b0},
    {u'온', 0b0},
    {u'옹', 0b0},
    {u'운', 0b0},
    {u'울', 0b0},
    {u'은', 0b0},
    {u'을', 0b0},
    {u'인', 0b0}
};

map<char16_t, unsigned int> etc_letters = {
    {'.', 0b0001000},
    {',', 0b0010000},
    {'+', 0b0},
    {'-', 0b0},
    {'=', 0b0},
    {'.', 0b0},
    {'(', 0b0},
    {')', 0b0},
    {' ', 0b0}
};

#endif
