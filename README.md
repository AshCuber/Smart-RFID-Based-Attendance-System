# Smart-RFID-Based-Attendance-System
Dev Board:   ESP8266 CP2102
RFID Reader/Writer: MFRC522
# Sheet Creation
1. Create a google sheet.
2. Go to extentions---> Apps Script.
3. Paste the code from the file 'Sheets_script.txt'.
4. Deploy the script as Web App.

# NodMCU Code Upload
1. Setup circuit according to the circuit diagram.
2. Upload the code of store_more_data_rfid.ino to NodeMCU (Make changes to personal data accordingly).
3. Bring RFID Card/tag near RC522.
4. Data will be sent to card.
5. Upload the code of to NodeMCU (Make changes to personal data accordingly).
6. Open the code 'Upload Data To Google Sheets.ino'.
7. Insert deployment id of your sheet in the respective part.
8. Insert wifi credentials.
9. Upload the code to NodeMCU
10. Bring RFID card/tag near RC522 module.
11. The lcd will show respective data in 4th block of card and upload data in all blocks to google sheet.
