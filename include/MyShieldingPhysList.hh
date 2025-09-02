#ifndef MyShieldingPhysList_h
#define MyShieldingPhysList_h 1

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

/**
 * @class MyShieldingPhysList
 * @brief CPNR_modular_sim을 위한 최종 물리 리스트.
 *
 * 표준 물리(EM, Decay, Optical)와 함께, ANNRI-Gd 모델이 포함된
 * 우리만의 커스텀 강입자 물리(MyHadronPhysics)를 등록합니다.
 */
class MyShieldingPhysList : public G4VModularPhysicsList
{
public:
    explicit MyShieldingPhysList(G4int verbose = 1);
    ~MyShieldingPhysList() override;

    void ConstructParticle() override;
    void ConstructProcess() override;
};

#endif
