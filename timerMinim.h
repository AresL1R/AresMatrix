class timerMinim
{
  public:
    timerMinim(uint32_t interval);				
    void setInterval(uint32_t interval);	
    boolean isReady();					        	
    void reset();						            	

  private:
    uint32_t _timer = 0;
    uint32_t _interval = 0;
};

timerMinim::timerMinim(uint32_t interval) {
  _interval = interval;
  _timer = millis();
}

void timerMinim::setInterval(uint32_t interval) {
  _interval = interval;
}

boolean timerMinim::isReady() {
  if (millis() - _timer >= _interval) {
    _timer = millis();
    return true;
  } else {
    return false;
  }
}

void timerMinim::reset() {
  _timer = millis();
}
