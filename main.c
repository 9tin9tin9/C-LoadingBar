#include "loadingbar.h"

int main(){
    LoadingBar lb = LoadingBar_new(10);
    for (int i = 0; i < 10; LoadingBar_update(&lb, ++i)){
        sleep(1);
    }
    return 0;
}
