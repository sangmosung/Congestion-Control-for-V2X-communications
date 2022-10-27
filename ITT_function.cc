#include <stdio.h>

// DCIB = Duration Channel Indicated as Busy, according to SAE J2945
// mod_ITT = modified ITT
// VD = vehicle density, all OBU
// DCIB는 첫번째 노드가 BSM을 보낸 이후 -> 마지막 노드의 BSM을 수신했을 때의 시간
float mod_ITT (float DCIB, int VD) {

    // 내부에서 정의되어야 하는 함수는 CBR, VD, X의 값이다.
    float vCBPMeasInt = 0.1; // according to SAE J2945
    float CBR = DCIB / vCBPMeasInt; // CBR = 0 ~ 1s
    float a;  // a = mod_ITT

    int congestion_level;
    if (CBR <= 0.2 || VD <= 100) {
        congestion_level = 1;
        printf("congestion_level: %d", congestion_level);
        return a = 0.1;
    } else if (0.2 < CBR <= 0.4 || 100 < VD <= 200) {
        congestion_level = 2;
        printf("congestion_level: %d", congestion_level);
        return a = 0.334;
    } else if (0.4 < CBR <= 0.6 || 300 < VD <= 400) {
        congestion_level = 3;
        printf("congestion_level: %d", congestion_level);
        return a = 0.5;
    } else if (0.6 < CBR <= 0.8 || 400 < VD <= 500) {
        congestion_level = 4;
        printf("congestion_level: %d", congestion_level);
        return a = 0.5;
    } else {
        congestion_level = 5;
        printf("congestion_level: %d", congestion_level);
        return a = 1;
    };

    return a;
}