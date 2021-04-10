void Chirp(int number)
{
  Serial.println("");
  Serial.print("cancelTimer");
  Serial.println(millis() - cancel_alarm.cancelTimer);
  if (number < 10) {
    if (millis() > cancel_alarm.cancelTimer ) {
      for (int i = 0; i < number; i++) {
        dit();
      }
    }
  }
  if (number == tempWarn) { // 'T' for temperature
    dah();
  }
}

int calculate_alarms()
{
  int alarmstate = 0;
  /*

    int             deltawarn = 40;
    int             deltacutoff = 80;
    long          high_cellwarn = 3500;
    long          high_cellcutoff = 3550;
    long          low_cellwarn = 2600;
    long          low_cellcutoff = 2500;
    long          high_bankwarn = 13850;
    long          high_bankcutoff = 14200;
    long          low_bankwarn = 12000;
    long          low_bankcutoff = 11500;
  */
  /*
     alarmstate = 0 do nothing
     alarmstate = 1 chirp
     alarmstate = 30 high alarm and disconnect
     alarmstate = 20 low alarm and disconnect

    int normal = 0;
    int lowchirp = 1;
    int chirp = 2;
    int highchirp = 3;

    const int lowcut = 20;
    const int lowon = 21;

    const int highcut = 30;
    const int highon = 31;
  */

  /* DEBUGPRINT3("Cell 1: ");
    DEBUGPRINTLN3(Cell[0]);
    DEBUGPRINT3("Cell 2: ");
    DEBUGPRINTLN3(Cell[1]);
    DEBUGPRINT3("Cell 3: ");
    DEBUGPRINTLN3(Cell[2]);
    DEBUGPRINT3("Cell 4: ");
    DEBUGPRINTLN3(Cell[3]);

  */
#ifdef TESTING
  DEBUGPRINTLN3();
  DEBUGPRINTLN3("---Calculate Alarms---");
  DEBUGPRINT3("bank: ");
  DEBUGPRINTLN3(bank);
  DEBUGPRINT3("highcellv: ");
  DEBUGPRINTLN3(highcellv);
  DEBUGPRINT3("lowcellv: ");
  DEBUGPRINTLN3(lowcellv);
  DEBUGPRINT3("Delta: ");
  DEBUGPRINTLN3(delta);
#endif

  if (bank > high_bankwarn) {
    alarmstate = highchirp;
    DEBUGPRINT3("high bank warnv: ");
    DEBUGPRINTLN3(bank);
    DEBUGPRINT3("hichirp: ");
    DEBUGPRINTLN3(alarmstate);
  }

  if (highcellv > high_cellwarn) {
    alarmstate = highchirp;
    DEBUGPRINT3("hi warn: ");
    DEBUGPRINTLN3(highcellv);
  }

  if (lowcellv < low_cellwarn) {
    alarmstate = lowchirp;
    DEBUGPRINT3("low warn: ");
    DEBUGPRINTLN3(lowcellv);
  }

  if (bank < low_bankwarn) {
    alarmstate = lowchirp;
    DEBUGPRINT3("lowBank: ");
    DEBUGPRINTLN3(bank);
  }

  if  (delta > deltawarn) {
    alarmstate = chirp;
    DEBUGPRINT3("deltawarn: ");
    DEBUGPRINTLN3(delta);
  }

  if (delta > deltacutoff) {
    DEBUGPRINT3("Delta Cutoff @: ");
    DEBUGPRINTLN3(delta);

    if (bank > 13200) {
      alarmstate = highcut;
      DEBUGPRINT3("Charging Delta: ");
      DEBUGPRINTLN3(alarmstate);
    }
    else
    {
      DEBUGPRINT3("Discharging Delta: ");
      DEBUGPRINTLN3(alarmstate);
      alarmstate = lowcut;
    }
  }

  if (bank > high_bankcutoff) {
    alarmstate = highcut;
    DEBUGPRINT3("Hicut ");
    DEBUGPRINTLN3(alarmstate);
  }

  if (highcellv > high_cellcutoff) {
    alarmstate = highcut;
    DEBUGPRINT3("HighCell Cut: ");
    DEBUGPRINT3(highcellv);
    DEBUGPRINT3(" Limit: ");
    DEBUGPRINTLN3(high_cellcutoff);
    DEBUGPRINTLN3(alarmstate);
  }

  if (lowcellv < low_cellcutoff) {
    alarmstate = lowcut;
    DEBUGPRINT3("lo cutoff: ");
    DEBUGPRINTLN3(lowcellv);
  }

  if (bank < low_bankcutoff) {
    alarmstate = lowcut;
    DEBUGPRINT3("lowcut: ");
    DEBUGPRINTLN3(bank);
    DEBUGPRINT3("lowcut alarm: ");
    DEBUGPRINTLN3(alarmstate);
  }

  xQueueSend(ChirpQueue, &alarmstate, 0);

  snprintf (msg, 75, "%d", alarmstate);
  MQ_Publish(ALARM, msg);

  deltaSum = deltaSum + delta;
  snprintf (msg, 75, "%d", deltaSum);
  MQ_Publish(DELTASUM, msg);

  // check for 'diverging Delta' fault
  if (delta > deltawarn) {

    if (deltaSum == 0)
    {
      deltaSum = delta;
    }
    else
    {
      if (delta > (deltaSum * 2))
      {
        // ESP.restart();
        Serial.println("");
        Serial.print("Delta:");
        Serial.println(deltaSum);
        Serial.println("");
        snprintf (msg, 75, "%d", deltaSum);
        MQ_Publish(DELTASUM, msg);
        deltaSum = delta;
      }
    }
  }
  else
  {
    deltaSum = 0;
  }
  return alarmstate;
}

void runalarms(int alarmcode, int oldAlarmCode) {

  if ((alarmcode == lowcut ) || (alarmcode == highcut))
  {
    alarmCount = alarmCount + 1;
    Serial.println("Alarm Cut");
  }
  snprintf (msg, 75, "%d", alarmCount);
  MQ_Publish("bps/alarmcount", msg);

  //Action required
  //Only do the cutoff if it has not worked
  if ((alarmcode == lowcut)  && (alarmCount > alarmLimit)) {
    relays(lowcut);
    alarmCount = 0;
  }
  if ((alarmcode == highcut) && (alarmCount > alarmLimit)) {
    relays(highcut);
    alarmCount = 0;
  }

}
