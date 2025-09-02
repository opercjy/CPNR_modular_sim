#ifndef MyHadronPhysics_h
#define MyHadronPhysics_h 1

#include "G4VPhysicsConstructor.hh"
#include "globals.hh"

/**
 * @class MyHadronPhysics
 * @brief 표준 강입자 물리 모델에 ANNRI-Gd 중성자 포획 모델을 추가하는 클래스
 *
 * G4VPhysicsConstructor를 상속받아, 물리 리스트에 부품처럼 끼워넣을 수 있는
 * 독립적인 물리 모듈입니다.
 */
class MyHadronPhysics : public G4VPhysicsConstructor
{
public:
    MyHadronPhysics(G4int verbose = 1);
    ~MyHadronPhysics() override;

    // 이 클래스의 핵심: 입자들을 생성하고 물리 프로세스를 등록하는 메소드
    void ConstructParticle() override;
    void ConstructProcess() override;

private:
    G4int fVerbose;
};

#endif
