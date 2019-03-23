# Digital Business card

Using a NodeMCU to create a digital business card.

- Created using a access point and a captive portal.

## Usage:
People that want your business card, get there smartphone and connect to the wifi (wireless access point) of the NodeMCU and enter (automatically) the captive portal and screenshot the business card. They will see a small website that represents as your business card.
## Installation:

- Create a little website with your personal information (must be in html) and put it in a "data" folder
  - Your project tree looks like this:
    - business_card
      - data
        - index.html
        - style.css
        - etc.
      - business_card.ino

- Change Hostname and SSID in business_card.ino

- Upload the data files using SPIFFS Upload tool

- Upload the business_card.ino to your NodeMCU

- Done

## Demo:

https://jrsvs.nl/digital-business-card
