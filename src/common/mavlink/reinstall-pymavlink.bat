@echo off
python -m pip uninstall -y pymavlink 

cd %~dp0\pymavlink

python setup.py clean --all

python setup.py bdist_wheel

cd dist

for %%a in (where pymavlink*.whl) Do Set wheel_path=%%a

python -m pip install %wheel_path%

cd %~dp0