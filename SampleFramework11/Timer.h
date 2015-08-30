//======================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//======================================================================

#pragma once

#include "PCH.h"

namespace SampleFramework11
{

class Timer
{

public:

    Timer();
    ~Timer();

    void Update();

    INT64 ElapsedSeconds() const;
    float ElapsedSecondsF() const;
    double ElapsedSecondsD() const;
    INT64 DeltaSeconds() const;
    float DeltaSecondsF() const;
    double DeltaSecondsD() const;

    INT64 ElapsedMilliseconds() const;
    float ElapsedMillisecondsF() const;
    double ElapsedMillisecondsD() const;
    INT64 DeltaMilliseconds() const;
    float DeltaMillisecondsF() const;
    double DeltaMillisecondsD() const;

    INT64 ElapsedMicroseconds() const;
    float ElapsedMicrosecondsF() const;
    double ElapsedMicrosecondsD() const;
    INT64 DeltaMicroseconds() const;
    float DeltaMicrosecondsF() const;
    double DeltaMicrosecondsD() const;

protected:

    INT64 frequency;
    double frequencyD;

    INT64 elapsed;
    INT64 delta;

    float elapsedF;
    float deltaF;

    double elapsedD;
    double deltaD;

    INT64 elapsedSeconds;
    INT64 deltaSeconds;

    float elapsedSecondsF;
    float deltaSecondsF;

    double elapsedSecondsD;
    double deltaSecondsD;

    INT64 elapsedMilliseconds;
    INT64 deltaMilliseconds;

    float elapsedMillisecondsF;
    float deltaMillisecondsF;

    double elapsedMillisecondsD;
    double deltaMillisecondsD;

    INT64 elapsedMicroseconds;
    INT64 deltaMicroseconds;

    float elapsedMicrosecondsF;
    float deltaMicrosecondsF;

    double elapsedMicrosecondsD;
    double deltaMicrosecondsD;
};

}