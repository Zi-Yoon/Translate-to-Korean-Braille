/*************************************************/
// Name		:	brail_print.cpp
// Author	:	An Jiyoon (Ziyoon)
/*************************************************/

#include "brail_print.h"

// 빈 점자 무시
bool ignore_empty(int binary) {
    if (binary == 0)
        return false;
    else
        return true;
}

void calculate_sol(unsigned int binary) {
    sol.bit_6 = binary % 2;
    binary = binary / 2;
    sol.bit_5 = binary % 2;
    binary = binary / 2;
    sol.bit_4 = binary % 2;
    binary = binary / 2;
    sol.bit_3 = binary % 2;
    binary = binary / 2;
    sol.bit_2 = binary % 2;
    binary = binary / 2;
    sol.bit_1 = binary % 2;
}

void move_sol() {
    digitalWrite(BRAIL_1, !sol.bit_1);
    digitalWrite(BRAIL_2, !sol.bit_2);
    digitalWrite(BRAIL_3, !sol.bit_3);
    digitalWrite(BRAIL_4, !sol.bit_4);
    digitalWrite(BRAIL_5, !sol.bit_5);
    digitalWrite(BRAIL_6, !sol.bit_6);
    while(1) {
        if (digitalRead(NEXT) == 1)
            break;
    }
}

void off_sol() {
    digitalWrite(BRAIL_1, 1);
    digitalWrite(BRAIL_2, 1);
    digitalWrite(BRAIL_3, 1);
    digitalWrite(BRAIL_4, 1);
    digitalWrite(BRAIL_5, 1);
    digitalWrite(BRAIL_6, 1);
}

// 점자 표시
void show_braille(int len) {
    for (int i = 0; i < len; i++) {
        if (ignore_empty(braille[i].front_1)) {
            calculate_sol(braille[i].front_1);
            move_sol();
        }
        if (ignore_empty(braille[i].front_2)) {
            calculate_sol(braille[i].front_2);
            move_sol();
        }
        if (ignore_empty(braille[i].mid_1)) {
            calculate_sol(braille[i].mid_1);
            move_sol();
        }
        if (ignore_empty(braille[i].mid_2)) {
            calculate_sol(braille[i].mid_2);
            move_sol();
        }
        if (ignore_empty(braille[i].end)) {
            calculate_sol(braille[i].end);
            move_sol();
        }
    }
    digitalWrite(BRAIL_1, 0);
    digitalWrite(BRAIL_2, 0);
    digitalWrite(BRAIL_3, 0);
    digitalWrite(BRAIL_4, 0);
    digitalWrite(BRAIL_5, 0);
    digitalWrite(BRAIL_6, 0);
}

// UTF-16 형식으로 변환
u16string utf_8to16() {
    int new_txt = 0;
    string line;
    while (new_txt == 0) {
        ifstream file("medicine_name.txt");
        getline(file, line);
        name_len = line.length();
        if (line != prev_line) {
            new_txt = 1;
        }
    }
    wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    u16string u16_line = converter.from_bytes(line);
    prev_line = line;
    return u16_line;
}

// front-mid-end-(end_twice)로 분리
// 한글 유니코드 범위 : 0xAC00 ~ 0xD7A3
// 유니코드 값 조정
// 숫자는 number에 저장
Hangul decompose_kor(char16_t ch) {
    Hangul hangul;
    if(ch >= 0x3131 && ch <= 0x3163) {
        hangul.front = ch;
        hangul.mid = 0;
        hangul.end = 0;
    }
    else if(ch >= 0xAC00 && ch <= 0xD7A3) {
        int ch_int = ch - 0xAC00;
        int end_idx = ch_int % 28;
        int mid_idx = ((ch_int - end_idx) / 28) % 21;
        int front_idx = ((ch_int - end_idx) / 28) / 21;
        hangul.front = front_list.at(front_idx);
        hangul.mid =   mid_list.at(mid_idx);
        hangul.end =   end_list.at(end_idx);
    }
    else if(ch >= 0x30 && ch <= 0x39){
        hangul.number = ch - 0x30;
        hangul.front = 0;
        hangul.mid =   0;
        hangul.end =   0;
    }
    else {
        hangul.front = 0;
        hangul.mid =   0;
        hangul.end =   0;
    }
    return hangul;
}

// 점자로 변경 후에 구조체에 저장
Braille convert_to_braille(Hangul hangul) {
    Braille braille;
    if (hangul.number != -1) {
        braille.front_1 = number_start;
        braille.front_2 = numbers[static_cast<char>(hangul.number)]; // 정적 캐스팅
    }
    else {
        if(front_letters[hangul.front] >= 0b1000000) {
            braille.front_1 = 0b0000001;
            braille.front_2 = front_letters[hangul.front];
        }
        else {
            braille.front_1 = 0;
            braille.front_2 = front_letters[hangul.front];
        }
        if (mid_letters[hangul.mid] >= 0b1000000) {
            braille.mid_1 = mid_letters[hangul.mid];
            braille.mid_2 = 0b0111010;
        }
        else {
            braille.mid_1 = mid_letters[hangul.mid];
            braille.mid_2 = 0;
        }
        if (!hangul.end) {
            braille.end = end_letters[hangul.end];
        }
    }
    return braille;
}

// 문장, 단어를 한 글자 단위로 분리 후 배열에 저장
void make_braille(u16string medicine) {
    int index = 0;
    for (size_t i = 0; i < medicine.size();) {
        Hangul hangul;
        int len = 1;
        // 서로게이트 페어 체크
        if (medicine[i] >= 0xD800 && medicine[i] <= 0xDBFF) {
            len = 2;
        }
        u16string char_str = medicine.substr(i, len);
        hangul = decompose_kor(char_str[0]);
        braille[index] = convert_to_braille(hangul);
        i += len;
        index++;
    }
}

/**********************Main***********************/
void setup() {
    wiringPiSetup();
    pinMode(BRAIL_1, OUTPUT);
    pinMode(BRAIL_2, OUTPUT);
    pinMode(BRAIL_3, OUTPUT);
    pinMode(BRAIL_4, OUTPUT);
    pinMode(BRAIL_5, OUTPUT);
    pinMode(BRAIL_6, OUTPUT);
    pinMode(NEXT   , INPUT );
}

void loop() {
    make_braille(utf_8to16());
    show_braille(name_len);
}

int main() {
    setup();
    loop();
    return 0;
}