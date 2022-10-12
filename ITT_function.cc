#include <stdio.h>

// ITT는 기존에 설정된 ITT값을 넣는다.
// TODO: ITT값을 기존의 코드에서 어떤것으로 넣어야 할지 결정해야 한다. 
// Solution(Broadcast): ITT = (0.02*n + 0.015) + (0.02*n + 0.01);
int mod_ITT  (float ITT) {
    // 내부에서 정의되어야 하는 함수는 CBR, VD, X의 값이다. 
    float CBR;
    int VD;
    float X;  // X = CBR x VD
    float a;  // a는 변경된 ITT의 값(최종 출력할 숫자)

    // CBR = 메시지를 전부 보내는 시간 / 1초(설정된 관측시간)
    // TODO: 메시지 보내는 시간 어떤것을 받아올 것인지?, 관측시간은 1초로 설정하면 괜찮은지 확인 필요
    CBR = 0.8;

    // 한 그룹내의 차량 대수 or 총 차량 대수(10~30)
    // TODO: 차량 대수를 결정하는 함수를 어떤것을 받아올 것인가?
    VD = (MAX_NODE - 1);  //  RSU node 1대 제외

    X = CBR * VD;

    // if문을 사용하여(X범위에 따른 mod_ITT 값 도출)
    if (X <= 6.5) {
        return a = 100;
    } else if (6.5 < X <= 14) {
        return a = 334;
    } else if (14 < X <= 25.5) {
        return a = 500;
    } else {
        return a = 1000;
    };

    return a;
}