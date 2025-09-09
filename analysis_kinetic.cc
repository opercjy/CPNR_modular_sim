// analysis_kinetic.C
// CPNR_modular_sim.root 파일을 읽어
// 에너지 증착을 일으킨 모든 입자들의 운동에너지 분포를 그리는 스크립트

void analysis_kinetic() {
    // 1. ROOT 파일과 Hits TTree를 엽니다.
    TFile *file = TFile::Open("build/cpnr_modular_sim.root", "READ");
    if (!file || file->IsZombie()) {
        std::cout << "Error: Cannot open root file." << std::endl;
        return;
    }
    TTree *tree = (TTree*)file->Get("Hits");

    // 2. TTree에서 kineticEnergy_MeV 데이터를 읽어올 변수를 선언하고 브랜치와 연결합니다.
    double kineticEnergy_MeV;
    tree->SetBranchAddress("kineticEnergy_MeV", &kineticEnergy_MeV);

    // 3. 결과를 담을 히스토그램을 생성합니다. (0~10 MeV 범위를 500개 bin으로)
    TH1F *h_kinetic = new TH1F("h_kinetic",
                               "Kinetic Energy of Particles Creating Hits;Kinetic Energy (MeV);Entries",
                               500, 0, 10);

    // 4. TTree의 모든 엔트리를 순회하며 히스토그램을 채웁니다.
    long long nEntries = tree->GetEntries();
    for (long long i = 0; i < nEntries; ++i) {
        tree->GetEntry(i);
        h_kinetic->Fill(kineticEnergy_MeV);
    }

    // 5. 히스토그램을 그리고 캔버스를 표시합니다.
    TCanvas *c1 = new TCanvas("c1", "Kinetic Energy Analysis", 800, 600);
    c1->SetLogy(); // Y축을 로그 스케일로 설정하여 넓은 범위의 값을 보기 용이하게 합니다.
    h_kinetic->Draw();
}
