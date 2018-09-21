# COOL FLASHER

With coolFlasher.sh you can send config to the shadow when you flash your board !


## Prerequisites:

 - aws cli
If you haven't, contact your admin for credentials. Then follow those instructions :
   - `pip install awscli`
   - `aws configure`: https://docs.aws.amazon.com/cli/latest/userguide/cli-chap-welcome.html
- your board is plugged and in _load_ mode
- platformIO

You always need to use option -e [...] for environnement and you can use -s [...] to flash SPIFFS (configuration) if you use this option you need to give your macAddress ([A-Z0-9]) ex: 32AEA4583D84.

## Example :

I want to flash only the code:

`flasher/flasher.sh -e debug`

i want to flash the code, the configuration and to send it to the shadow:

`flash/flasher.sh -e debug -s 32AEA4583D84`

