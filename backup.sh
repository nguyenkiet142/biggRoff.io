TOKEN=7322952979:private
CHAT_ID=1797791229

curl https://api.telegram.org/bot$TOKEN/sendDocument \
    --form chat_id=$CHAT_ID \
    --form document=@/root/rysteria/MasterServer/database.json
