**P1-port smart meter module**

The following components are only needed if the smart meter uses DSMR version 2.2. For higher versions these components are not needed:

J3 USB connector

D1 Shottky diode

D2 Shottky diode, this diode must be replaced by a straight wire for higher DSMR versions.

J2 Jumper

In DSMR2.2 the USB connector is used to provide 5V, to be delivered by a 5V USB Power supply

For DSMR2.2 a jumper must be placed on J2, because the serial connection to the DSMR2.2 meter uses different settings. E.g. a baudrate of 9600 baud in DSMR2.2, while in highet DSMR versions the baudrate is 115200 baud.

**List of components used:**

J1 Header + cable with RJ12 connector, RJ12 naar 6 Pins Dupont-Jumper Adapter - P1 Kabel Compatible {e.g. [https://www.tinytronics.nl/shop/nl/kabels/adapters/rj12-naar-6-pins-dupont-jumper-adapter](https://www.tinytronics.nl/shop/nl/kabels/adapters/rj12-naar-6-pins-dupont-jumper-adapter))

J2 2-pole header (e.g. [https://www.tinytronics.nl/shop/nl/connectoren/pin-header/40-pins-header-male](https://www.tinytronics.nl/shop/nl/connectoren/pin-header/40-pins-header-male))

J3 USB-aansluitstekker printmontage Stekker, recht Typ A USB-stekker type A 774842 TRU COMPONENTS (e.g. [https://www.conrad.nl/p/usb-aansluitstekker-printmontage-stekker-recht-typ-a-usb-stekker-type-a-774842-tru-components-inhoud-1-stuks-1567162?searchTerm=USB-aansluitstekker%20printmontage%20Stekker&amp;searchType=suggest&amp;searchSuggest=product](https://www.conrad.nl/p/usb-aansluitstekker-printmontage-stekker-recht-typ-a-usb-stekker-type-a-774842-tru-components-inhoud-1-stuks-1567162?searchTerm=USB-aansluitstekker%20printmontage%20Stekker&amp;searchType=suggest&amp;searchSuggest=product))

U1 WeMos D1 Mini, Wemos D1 Mini V3 - ESP8266 - CH340 (e.g. [https://www.tinytronics.nl/shop/nl/arduino/wemos/wemos-d1-mini-v2-esp8266-12f-ch340](https://www.tinytronics.nl/shop/nl/arduino/wemos/wemos-d1-mini-v2-esp8266-12f-ch340))

Q1 BC546 (e.g. [https://www.tinytronics.nl/shop/nl/componenten/transistor-fet/npn-transistor-bc547](https://www.tinytronics.nl/shop/nl/componenten/transistor-fet/npn-transistor-bc547))

D1, D2 1N5819 (e.g. [https://www.conrad.nl/p/stmicroelectronics-skottky-diode-gelijkrichter-1n5819-do-41-40-v-enkelvoudig-155460](https://www.conrad.nl/p/stmicroelectronics-skottky-diode-gelijkrichter-1n5819-do-41-40-v-enkelvoudig-155460))

R1, R2 Resistor 1k (e.g. [https://www.conrad.nl/p/yageo-mf0207fte52-1k-metaalfilmweerstand-1-k-axiaal-bedraad-0207-06-w-1-1-stuks-1417606](https://www.conrad.nl/p/yageo-mf0207fte52-1k-metaalfilmweerstand-1-k-axiaal-bedraad-0207-06-w-1-1-stuks-1417606))

R3 Resistor 3k3 (e.g. [https://www.conrad.nl/p/yageo-mf0207fte52-3k3-metaalfilmweerstand-33-k-axiaal-bedraad-0207-06-w-1-1-stuks-1417733](https://www.conrad.nl/p/yageo-mf0207fte52-3k3-metaalfilmweerstand-33-k-axiaal-bedraad-0207-06-w-1-1-stuks-1417733))