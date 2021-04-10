void ADC_Setup() {
  MCP342x::generalCallReset();

  delay(1); // MC342x needs 300us to settle, wait 1ms

  // Check device present
  Wire.requestFrom(address, (uint8_t)1);
  if (!Wire.available()) {
    Serial.print("No device found at address ");
    Serial.println(address, HEX);

    /* ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
      ads.begin();
    */

    //ads.setSPS(ADS1115_DR_64SPS);
  }
}

void readVoltage()
{
  long adc0 = 0, adc1 = 0, adc2 = 0, adc3 = 0;
  uint16_t ADC[] = {0, 0, 0, 0};
  char mymsg[50];
  int iterations = 9;
  unsigned long uptimeStart;
  uptimeStart =  millis();

  MCP342x::Config status;

  uint8_t err = adc.convertAndRead(MCP342x::channel1, MCP342x::oneShot,
                                   MCP342x::resolution16, MCP342x::gain1,
                                   1000000, adc0, status);

  err = adc.convertAndRead(MCP342x::channel2, MCP342x::oneShot,
                           MCP342x::resolution16, MCP342x::gain1,
                           1000000, adc1, status);

  err = adc.convertAndRead(MCP342x::channel3, MCP342x::oneShot,
                           MCP342x::resolution16, MCP342x::gain1,
                           1000000, adc2, status);

  err = adc.convertAndRead(MCP342x::channel4, MCP342x::oneShot,
                           MCP342x::resolution16, MCP342x::gain1,
                           1000000, adc3, status);

  /*
     Convert to Voltage
  */
  ADC[0] = adc0 * 2 * 2048 / 32768 ;
  ADC[1] = adc1 * 4 * 2048 / 32768 ;
  ADC[2] = (adc2 - 71) * 6 * 2048 / 32768 ;
  ADC[3] = (adc3 + 238) * 8 * 2048 / 32768 ;

  ADC[0] = ADC[0] + 24; //+ 22;//27;
  ADC[1] = ADC[1] + 69; // + 120;//60;
  ADC[2] = ADC[2] + 117; //+ 144;//57;
  ADC[3] = ADC[3] + 18;//70;

#ifdef TESTING
  snprintf (msg, 50, " % d", ADC[0] );
  MQ_Publish( "bps/adc0", msg);
  snprintf (msg, 50, " % d", ADC[1]  );
  MQ_Publish( "bps/adc1", msg);
  snprintf (msg, 50, " % d", ADC[2]   );
  MQ_Publish( "bps/adc2", msg);
  snprintf (msg, 50, " % d", ADC[3] );
  MQ_Publish( "bps/adc3", msg);
#endif

  Cell[0] = ADC[0];// + 26; //- 8 ;
  Cell[1] = ADC[1] - ADC[0];// + 112;
  Cell[2] = ADC[2] - ADC[1];// - 35 ; //- 9;
  Cell[3] = ADC[3] - ADC[2];// + 45 ; //+ 33;
  bank = ADC[3] ;//+ 15;

#ifdef TESTINGVOLTAGE
  Cell[0] = testCell[0] ;
  Cell[1] = testCell[1] ;
  Cell[2] = testCell[2] ;
  Cell[3] = testCell[3] ;
  bank = testCell[0] + testCell[1] + testCell[2] + testCell[3];
#endif

  highcell = CellMax(Cell, sizeof(Cell) / sizeof(Cell[0]));
  lowcell = CellMin(Cell, sizeof(Cell) / sizeof(Cell[0]));
  cellsum = Cell[0] + Cell[1] + Cell[2] + Cell[3];
  highcellv = Cell[highcell];
  lowcellv = Cell[lowcell];
  delta = highcellv - lowcellv;

  if (millis() > 10000) {
    if (millis() > reportTimer)
    {
      reportTimer = millis() + 5000;

      snprintf (msg, 50, " % d", Cell[0]);
      MQ_Publish(CELL1, msg);
      DEBUGPRINTLN3(msg);
      snprintf (msg, 50, " % d", Cell[1]);
      MQ_Publish(CELL2, msg);
      DEBUGPRINTLN3(msg);
      snprintf (msg, 50, " % d", Cell[2]);
      MQ_Publish(CELL3, msg);
      DEBUGPRINTLN3(msg);
      snprintf (msg, 50, " % d", Cell[3]);
      MQ_Publish(CELL4, msg);
      DEBUGPRINTLN3(msg);

      snprintf (msg, 50, " % d", bank);
      MQ_Publish(BANK, msg);
      DEBUGPRINTLN3(msg);
      snprintf (msg, 50, " % d", delta);
      MQ_Publish(DELTA, msg);
      snprintf (msg, 50, " % d", highcell + 1);
      MQ_Publish(HICELL, msg);
      snprintf (msg, 50, " % d", lowcell + 1);
      MQ_Publish(LOCELL, msg);
      snprintf (msg, 50, " % d", cellsum + 1);
      MQ_Publish(CELLSUM, msg);

      snprintf (msg, 50, " %u", (millis() / 1000));
      MQ_Publish(UPTIME, msg);

    }
  }
  snprintf (msg, 50, " %u", (millis() - uptimeStart));
  MQ_Publish("bps/readtime", msg);

}

int CellMax( int arry[], int arrysize) {
  int result = 0;
  int i;
  for (i = 1; i < arrysize ; i++) {
    /*    DEBUGPRINT3("arry[i] : ");
      DEBUGPRINT3(arry[i - 1]);
      DEBUGPRINT3(" result : ");
      DEBUGPRINTLN3(arry[result]);
    */
    if (arry[i] > arry[result]) {
      result = i;
    }
  }
  return result;
}

int CellMin( int arry[], int arrysize) {
  int result = 0;
  int i ;
  for ( i = 1; i < arrysize ; i++) {
    /*    DEBUGPRINT3("arry[i] : ");
      DEBUGPRINT3(arry[i - 1]);
      DEBUGPRINT3(" result : ");
      DEBUGPRINTLN3(arry[result]);
    */
    if (arry[i] < arry[result]) {
      result = i;
    }
  }
  return result;
}
