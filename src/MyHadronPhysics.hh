#ifndef MyHadronPhysics_h
#define MyHadronPhysics_h 1

#include "G4VPhysicsConstructor.hh"
#include "globals.hh"

class G4HadronPhysicsFTFP_BERT_HP;

class MyHadronPhysics : public G4VPhysicsConstructor
{
public:
    MyHadronPhysics(G4int verbose = 1);
    ~MyHadronPhysics() override;

    void ConstructParticle() override;
    void ConstructProcess() override;

private:
    // 표준 물리 리스트를 재사용하여 중성자 외 다른 입자들의 물리를 구성
    std::unique_ptr<G4HadronPhysicsFTFP_BERT_HP> fStdPhysics;
};

#endif
