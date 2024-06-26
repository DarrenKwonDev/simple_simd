﻿#include <iostream>
#include <string>
#include <array>
#include <memory>

#include <intrin.h>
#include <random>
#include <chrono>


void ASM_get_cpu_name()
{
    uint32_t data[4] = { 0 };

    //-----------------------------------------------
    // MSVC x86 에서는 __asm 사용
    // MSVC x64 에서는 어셈블리 직접 사용 불가. Intrinsic function을 사용함.
    // gcc, clang 등은 asm 혹은 __asm__ 사용
    // 아래 코드는 x86 MSVC 환경에서 사용됨.
    //-----------------------------------------------
    __asm {
        cpuid;
        mov data[0], ebx;
        mov data[4], edx;
        mov data[8], ecx;
    }

    std::cout << std::string((const char*)data) << "\n";
}

void SIMD_pair_wise()
{
    //-----------------------------------------------
    // pair wise SIMD operation.
    float f1[] = { 1.f, 2.f, 3.f, 4.f };
    float f2[] = { 5.f, 4.f, 3.f, 2.f };
    float res[4] = { 0.f };

    // SSE instructions
    // https://en.wikipedia.org/wiki/Streaming_SIMD_Extensions#SSE_instructions
    __asm {
        movups xmm1, f1;        // xmm1 레지스터로 f1를 이동시킨다
        movups xmm2, f2;        // xmm2 레지스터로 f2를 이동시킨다
        mulps xmm1, xmm2;       // pair wise 곱셈을 SIMD로 처리한다.
        movups res, xmm1;       // res에 복사한다.
    }

    for (size_t i = 0; i < 4; i++)
    {
        std::cout << res[i] << "\t";
    }
    std::cout << "\n";
}


void Intrinsic_get_cpu_name()
{
    int nCPUInfo[4];
    __cpuid(nCPUInfo, 1);
}


void SIMD_intrinsic()
{
    //-----------------------------------------------
    // 해당 함수를 사용하려면 '고급 명령 집합' 에서 SSE를 활성화 해야한다.
    // 추가로, #include <intrin.h> 도 필요.
    
    auto a = _mm_set_ps(1.f, 2.f, 3.f, 4.f);
    auto b = _mm_set_ps(4.f, 3.f, 2.f, 1.f);
    auto c = _mm_add_ps(a, b); // 덧셈 연산 SIMD
    
    float res[4];
    _mm_storeu_ps(res, c);

    for (size_t i = 0; i < 4; i++)
    {
        std::cout << res[i] << "\t";
    }
    std::cout << "\n";
}

void optimization()
{
    const int length = 1024 * 1024 * 64;
    
    //-----------------------------------------------
    // 의도 : a * b + c 를 SIMD로 처리하여 res에 저장
    float* a = new float[length];
    float* b = new float[length];
    float* c = new float[length];
    float* res = new float[length];

    //-----------------------------------------------
    // random 난수를 채운다.
    std::mt19937_64 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0, 1);
    for (size_t i = 0; i < length; i++)
    {
        a[i] = dist(rng); b[i] = dist(rng); c[i] = dist(rng);
    }

    //-----------------------------------------------
    // SISD로 처리 될 것이다.
    // 그러나 optimization 레벨을 높이면 알아서 SIMD로 적용되고, loop는 vectorized  된다.
    // 벡터화는 CPU의 벡터 레지스터와 SIMD 명령어를 활용하여, 하나의 명령어로 여러 데이터를 병렬로 처리하는 최적화 기술입니다.
    // 그러니까 loop를 다 돌지 않고 한 방에 "빡" 된다는 것이다.
    auto begin = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < length; i++)
    {
        res[i] = a[i] * b[i] + c[i];
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "took : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;

    delete[] a; delete[] b; delete[] c; delete[] res;
}

int main()
{
    //-----------------------------------------------
    // examles
    ASM_get_cpu_name();
    SIMD_pair_wise();
    SIMD_intrinsic();

    //-----------------------------------------------
    // examples 2
    // 의도.
    // a * b + c 를 SIMD로 처리하고 싶습니다.
    optimization();



    return 0;
}

