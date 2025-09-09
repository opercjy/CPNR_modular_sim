// analysis.C
// CPNR_modular_sim.root 파일을 읽어 이벤트별 총 에너지 증착 스펙트럼을 그리는 스크립트

void analysis() {
    // 1. ROOT 파일과 Hits TTree를 엽니다.
    TFile *file = TFile::Open("build/cpnr_modular_sim.root", "READ");
    if (!file || file->IsZombie()) {
        std::cout << "Error: Cannot open root file." << std::endl;
        return;
    }
    TTree *tree = (TTree*)file->Get("Hits");

    // 2. TTree에서 데이터를 읽어올 변수들을 선언하고 브랜치와 연결합니다.
    int eventID;
    double energyDeposit_MeV;
    tree->SetBranchAddress("eventID", &eventID);
    tree->SetBranchAddress("energyDeposit_MeV", &energyDeposit_MeV);

    // 3. 결과를 담을 히스토그램을 생성합니다. (0~12 MeV 범위를 600개 bin으로)
    TH1F *h_total_edep = new TH1F("h_total_edep", "Total Energy Deposit per Event;Energy (MeV);Entries", 600, 0, 12);

    // 4. TTree의 모든 엔트리를 순회하며 이벤트별로 에너지 증착을 합산합니다.
    long long nEntries = tree->GetEntries();
    double current_event_total_energy = 0;
    int previous_eventID = -1;

    tree->GetEntry(0);
    previous_eventID = eventID;

    for (long long i = 0; i < nEntries; ++i) {
        tree->GetEntry(i);

        if (eventID != previous_eventID) {
            // 이벤트 ID가 바뀌면, 이전 이벤트의 총 합산 에너지를 히스토그램에 채웁니다.
            if (current_event_total_energy > 0) {
                h_total_edep->Fill(current_event_total_energy);
            }
            // 다음 이벤트를 위해 변수들을 초기화합니다.
            previous_eventID = eventID;
            current_event_total_energy = 0;
        }
        current_event_total_energy += energyDeposit_MeV;
    }
    // 마지막 이벤트의 에너지를 채웁니다.
    if (current_event_total_energy > 0) {
        h_total_edep->Fill(current_event_total_energy);
    }

    // 5. 히스토그램을 그리고 캔버스를 표시합니다.
    TCanvas *c1 = new TCanvas("c1", "Analysis Result", 800, 600);
    h_total_edep->Draw();

    // 파일은 열어두어 사용자가 상호작용할 수 있도록 합니다.
    // file->Close(); // 필요시 주석 해제
}
