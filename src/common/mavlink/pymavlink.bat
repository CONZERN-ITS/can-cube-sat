
del /s /f /q generated

python -m pymavlink.tools.mavgen -o generated\mavlink --lang=C --wire-protocol=2.0 message_definitions\v1.0\its.xml