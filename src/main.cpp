// https://github.com/m5stack/M5Module-LLM/tree/dev/examples/SerialTextAssistant
// https://github.com/m5stack/M5Cardputer/tree/master/examples/Basic/keyboard/inputText
// この二つを組み合わせたもの
// スペシャルサンクス（というか作った人） @GOROman

#include <Arduino.h>

#include "M5Cardputer.h"
#include "M5GFX.h"
#include "M5ModuleLLM.h"

#define MODEL "qwen2.5-0.5b"

M5ModuleLLM module_llm;
String llm_work_id;
String received_question;
bool question_ok;

M5Canvas canvas(&M5Cardputer.Display);
String data = "> ";

void serialTXRX(const String &data);

void setup()
{
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setFont(&fonts::efontJA_16);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.drawRect(0, 0, M5Cardputer.Display.width(),
                               M5Cardputer.Display.height() - 28, YELLOW);

  M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - 4,
                               M5Cardputer.Display.width(), 4, YELLOW);

  canvas.setFont(&fonts::efontJA_16);
  canvas.setTextSize(1);
  canvas.createSprite(M5Cardputer.Display.width() - 8,
                      M5Cardputer.Display.height() - 36);
  canvas.setTextScroll(true);

  M5Cardputer.Display.drawString(data, 4, M5Cardputer.Display.height() - 24);

  /* Init module serial port */
  Serial2.begin(115200, SERIAL_8N1, 1, 2);

  /* Init module */
  module_llm.begin(&Serial2);

  /* Make sure module is connected */
  canvas.println(">> Check ModuleLLM connection..");
  while (1)
  {
    if (module_llm.checkConnection())
    {
      break;
    }
  }

  /* Reset ModuleLLM */
  canvas.println(">> Reset ModuleLLM..");
  canvas.pushSprite(4, 4);
  module_llm.sys.reset();

  /* Setup LLM module and save returned work id */
  canvas.println(">> Setup LLM..");
  canvas.pushSprite(4, 4);

  m5_module_llm::ApiLlmSetupConfig_t llm_config;
  llm_config.model = MODEL;
  llm_config.max_token_len = 1023;
  llm_config.prompt = "Please answer in Japanese.";
  llm_work_id = module_llm.llm.setup(llm_config);

  canvas.println(">> Setup finish");
  canvas.pushSprite(4, 4);
  canvas.println(">> Try send your question");
  canvas.pushSprite(4, 4);
}

void loop()
{
  M5Cardputer.update();
  if (M5Cardputer.Keyboard.isChange())
  {
    if (M5Cardputer.Keyboard.isPressed())
    {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

      for (auto i : status.word)
      {
        data += i;
      }

      if (status.del)
      {
        data.remove(data.length() - 1);
      }

      if (status.enter)
      {
        data.remove(0, 2);
        canvas.println(data);
        canvas.pushSprite(4, 4);
        serialTXRX(data);
        data = "> ";
      }

      M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - 28,
                                   M5Cardputer.Display.width(), 25,
                                   BLACK);

      M5Cardputer.Display.drawString(data, 4,
                                     M5Cardputer.Display.height() - 24);
    }
  }
}

void serialTXRX(const String &data)
{
  received_question = data;
  canvas.println("<< " + received_question);
  canvas.pushSprite(4, 4);
  canvas.setTextColor(YELLOW);
  canvas.setTextColor(WHITE);
  canvas.println(">> ");
  canvas.pushSprite(4, 4);

  /* Push question to LLM module and wait inference result */
  module_llm.llm.inferenceAndWaitResult(llm_work_id, received_question.c_str(), [](String &result)
                                        {
      /* Show result on screen and usb serial */
      canvas.print(result);
      canvas.pushSprite(4, 4); });

  /* Clear for next question */
  received_question.clear();

  canvas.println();
  canvas.pushSprite(4, 4);
}