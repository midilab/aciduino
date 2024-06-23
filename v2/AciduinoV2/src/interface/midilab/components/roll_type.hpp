struct RollType : PageComponent {

    uint8_t current_roll = 0;
    ROLL_TYPE rollTypes[7] = {FLAM_1, FLAM_2, FLAM_3, FLAM_4, FLAM_5, SUB_STEP_1, SUB_STEP_2};

    String options;
    void view() {
      uint8_t roll_type = rollTypes[current_roll]; //aciduino.seq.getRollType(aciduino.getSelectedTrack());
      if (roll_type <= FLAM_5) {
        options = "f"  + String(roll_type);
      } else {
        options = "s"  + String(roll_type - FLAM_5);
      }
      genericOptionView("roll", options, line, col, selected);
      // flam1, flam2, flam3, flam4, flam5, flam6. sub6, sub3, sub2?
    }

    void change(int16_t data) {
      //clearStackNote(aciduino.getSelectedTrack());
      current_roll = parseData(data, 0, 6, current_roll); //aciduino.seq.getRollType(aciduino.getSelectedTrack()));
      aciduino.seq.setRollType(aciduino.getSelectedTrack(), rollTypes[current_roll]);
    }
    
} rollTypeComponent;