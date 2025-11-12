const axios = require("axios");

// ---------- Blynk Token ----------
const BLYNK_TOKEN = "NSbjLXws4gyPkRfsPKUUOr_bPT1_TTUf";

// ---------- Virtual Pin APIs ----------
const API_TEMP = `https://blynk.cloud/external/api/get?token=${BLYNK_TOKEN}&V0`;
const API_HUM = `https://blynk.cloud/external/api/get?token=${BLYNK_TOKEN}&V1`;
const API_PM1 = `https://blynk.cloud/external/api/get?token=${BLYNK_TOKEN}&V4`;
const API_PM25 = `https://blynk.cloud/external/api/get?token=${BLYNK_TOKEN}&V5`;
const API_PM10 = `https://blynk.cloud/external/api/get?token=${BLYNK_TOKEN}&V6`;
const API_AQI = `https://blynk.cloud/external/api/get?token=${BLYNK_TOKEN}&V7`;
const API_MQ7 = `https://blynk.cloud/external/api/get?token=${BLYNK_TOKEN}&V2`;      // MQ7 -> Carbon Monoxide
const API_MQ135 = `https://blynk.cloud/external/api/get?token=${BLYNK_TOKEN}&V3`;    // MQ135 -> Toxic Gas

// ---------- Utility ----------
function parseBlynkValue(data) {
  if (Array.isArray(data)) return data[0];
  if (typeof data === "string") return data.trim();
  return data;
}

// ---------- Response Builder ----------
class ResponseBuilder {
  constructor() {
    this.resp = {
      version: "1.0",
      response: {
        shouldEndSession: true,
      },
    };
  }
  speak(text) {
    this.resp.response.outputSpeech = { type: "PlainText", text };
    return this;
  }
  withSimpleCard(title, content) {
    this.resp.response.card = { type: "Simple", title, content };
    return this;
  }
  getResponse() {
    return this.resp;
  }
}

// ---------- Intent Handlers ----------

const GetAirQualityHandler = {
  canHandle(handlerInput) {
    return (
      handlerInput.requestEnvelope.request.type === "IntentRequest" &&
      handlerInput.requestEnvelope.request.intent.name === "GetAirQualityIntent"
    );
  },
  async handle(handlerInput) {
    try {
      const res = await axios.get(API_AQI);
      const data = res.data.toString().trim(); // e.g. "AQI: 126 Unhealthy for Sensitive Groups"

      let aqiNumber = "unknown";
      let aqiCategory = "unknown";

      if (data.startsWith("AQI:")) {
        const parts = data.replace("AQI:", "").trim().split(" ");
        aqiNumber = parts.shift();
        aqiCategory = parts.join(" ");
      }

      return handlerInput.responseBuilder
        .speak(
          `The air quality index is ${aqiNumber}, which is ${aqiCategory}.`
        )
        .withSimpleCard("Air Quality", `AQI: ${aqiNumber}\n${aqiCategory}`)
        .getResponse();
    } catch (err) {
      console.error("❌ Error fetching AQI:", err);
      return handlerInput.responseBuilder
        .speak("Sorry, I couldn’t fetch the air quality right now.")
        .getResponse();
    }
  },
};

const GetTemperatureHandler = {
  canHandle(handlerInput) {
    return (
      handlerInput.requestEnvelope.request.type === "IntentRequest" &&
      handlerInput.requestEnvelope.request.intent.name === "GetTemperatureIntent"
    );
  },
  async handle(handlerInput) {
    try {
      const res = await axios.get(API_TEMP);
      const temp = parseBlynkValue(res.data);
      return handlerInput.responseBuilder
        .speak(`The temperature is ${temp} degrees Celsius.`)
        .withSimpleCard("Temperature", `${temp} °C`)
        .getResponse();
    } catch (err) {
      console.error("❌ Error fetching temperature:", err);
      return handlerInput.responseBuilder
        .speak("Sorry, I couldn’t fetch the temperature right now.")
        .getResponse();
    }
  },
};

const GetHumidityHandler = {
  canHandle(handlerInput) {
    return (
      handlerInput.requestEnvelope.request.type === "IntentRequest" &&
      handlerInput.requestEnvelope.request.intent.name === "GetHumidityIntent"
    );
  },
  async handle(handlerInput) {
    try {
      const res = await axios.get(API_HUM);
      const hum = parseBlynkValue(res.data);
      return handlerInput.responseBuilder
        .speak(`The humidity is ${hum} percent.`)
        .withSimpleCard("Humidity", `${hum} %`)
        .getResponse();
    } catch (err) {
      console.error("❌ Error fetching humidity:", err);
      return handlerInput.responseBuilder
        .speak("Sorry, I couldn’t fetch the humidity right now.")
        .getResponse();
    }
  },
};

const GetPMOneHandler = {
  canHandle(handlerInput) {
    return (
      handlerInput.requestEnvelope.request.type === "IntentRequest" &&
      handlerInput.requestEnvelope.request.intent.name === "GetPMOneIntent"
    );
  },
  async handle(handlerInput) {
    try {
      const res = await axios.get(API_PM1);
      const pm1 = parseBlynkValue(res.data);
      return handlerInput.responseBuilder
        .speak(
          `The PM one point zero concentration is ${pm1} micrograms per cubic meter.`
        )
        .withSimpleCard("PM1.0", `${pm1} µg/m³`)
        .getResponse();
    } catch (err) {
      console.error("❌ Error fetching PM1:", err);
      return handlerInput.responseBuilder
        .speak("Sorry, I couldn’t fetch PM one point zero right now.")
        .getResponse();
    }
  },
};

const GetPMTwoFiveHandler = {
  canHandle(handlerInput) {
    return (
      handlerInput.requestEnvelope.request.type === "IntentRequest" &&
      handlerInput.requestEnvelope.request.intent.name === "GetPMTwoFiveIntent"
    );
  },
  async handle(handlerInput) {
    try {
      const res = await axios.get(API_PM25);
      const pm25 = parseBlynkValue(res.data);
      return handlerInput.responseBuilder
        .speak(
          `The PM two point five concentration is ${pm25} micrograms per cubic meter.`
        )
        .withSimpleCard("PM2.5", `${pm25} µg/m³`)
        .getResponse();
    } catch (err) {
      console.error("❌ Error fetching PM2.5:", err);
      return handlerInput.responseBuilder
        .speak("Sorry, I couldn’t fetch PM two point five right now.")
        .getResponse();
    }
  },
};

const GetPMTenHandler = {
  canHandle(handlerInput) {
    return (
      handlerInput.requestEnvelope.request.type === "IntentRequest" &&
      handlerInput.requestEnvelope.request.intent.name === "GetPMTenIntent"
    );
  },
  async handle(handlerInput) {
    try {
      const res = await axios.get(API_PM10);
      const pm10 = parseBlynkValue(res.data);
      return handlerInput.responseBuilder
        .speak(
          `The PM ten concentration is ${pm10} micrograms per cubic meter.`
        )
        .withSimpleCard("PM10", `${pm10} µg/m³`)
        .getResponse();
    } catch (err) {
      console.error("❌ Error fetching PM10:", err);
      return handlerInput.responseBuilder
        .speak("Sorry, I couldn’t fetch PM ten right now.")
        .getResponse();
    }
  },
};

const API_CO_CATEGORY = `https://blynk.cloud/external/api/get?token=${BLYNK_TOKEN}&V9`;
const API_GAS_CATEGORY = `https://blynk.cloud/external/api/get?token=${BLYNK_TOKEN}&V10`;

// ---------- Carbon Monoxide ----------
const GetCarbonMonoxideHandler = {
  canHandle(handlerInput) {
    return (
      handlerInput.requestEnvelope.request.type === "IntentRequest" &&
      handlerInput.requestEnvelope.request.intent.name === "GetCarbonMonoxideIntent"
    );
  },
  async handle(handlerInput) {
    try {
      const [resValue, resCategory] = await Promise.all([
        axios.get(API_MQ7),
        axios.get(API_CO_CATEGORY),
      ]);

      const coValue = parseBlynkValue(resValue.data);
      const categoryData = parseBlynkValue(resCategory.data); // e.g. "CO: 4.2 ppm Good"

      // Extract details from category string
      let coPPM = coValue;
      let coCategory = "Unknown";

      const match = categoryData.match(/CO:\s*([\d.]+)\s*ppm\s*(.*)/i);
      if (match) {
        coPPM = match[1];
        coCategory = match[2];
      }

      return handlerInput.responseBuilder
        .speak(
          `The carbon monoxide level is ${coPPM} parts per million, which is ${coCategory}.`
        )
        .withSimpleCard("Carbon Monoxide", `${coPPM} ppm - ${coCategory}`)
        .getResponse();
    } catch (err) {
      console.error("❌ Error fetching CO data:", err);
      return handlerInput.responseBuilder
        .speak("Sorry, I couldn’t fetch the carbon monoxide level right now.")
        .getResponse();
    }
  },
};

// ---------- Toxic Gas ----------
const GetToxicGasHandler = {
  canHandle(handlerInput) {
    return (
      handlerInput.requestEnvelope.request.type === "IntentRequest" &&
      handlerInput.requestEnvelope.request.intent.name === "GetToxicGasIntent"
    );
  },
  async handle(handlerInput) {
    try {
      const [resValue, resCategory] = await Promise.all([
        axios.get(API_MQ135),
        axios.get(API_GAS_CATEGORY),
      ]);

      const gasValue = parseBlynkValue(resValue.data);
      const categoryData = parseBlynkValue(resCategory.data); // e.g. "Gas: 8.3 ppm Moderate"

      // Extract details
      let gasPPM = gasValue;
      let gasCategory = "Unknown";

      const match = categoryData.match(/Gas:\s*([\d.]+)\s*ppm\s*(.*)/i);
      if (match) {
        gasPPM = match[1];
        gasCategory = match[2];
      }

      return handlerInput.responseBuilder
        .speak(
          `The general gas concentration is ${gasPPM} parts per million, which is ${gasCategory}.`
        )
        .withSimpleCard("Toxic Gas", `${gasPPM} ppm - ${gasCategory}`)
        .getResponse();
    } catch (err) {
      console.error("❌ Error fetching toxic gas data:", err);
      return handlerInput.responseBuilder
        .speak("Sorry, I couldn’t fetch the toxic gas level right now.")
        .getResponse();
    }
  },
};


// ---------- Main Lambda Entrypoint ----------
exports.handler = async (event, context) => {
  const responseBuilder = new ResponseBuilder();
  const handlerInput = { requestEnvelope: event, responseBuilder };

  const handlers = [
    GetAirQualityHandler,
    GetTemperatureHandler,
    GetHumidityHandler,
    GetPMOneHandler,
    GetPMTwoFiveHandler,
    GetPMTenHandler,
    GetCarbonMonoxideHandler,
    GetToxicGasHandler,
  ];

  for (const handler of handlers) {
    if (handler.canHandle(handlerInput)) {
      return await handler.handle(handlerInput);
    }
  }

  // Default fallback
  return responseBuilder
    .speak("Sorry, I didn’t understand that request.")
    .getResponse();
};
