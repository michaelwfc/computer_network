# install

## install from local
```bash
# conda create -n pdf2md python=3.12
# conda activate pdf2md


cd pdf2md
which python
python -m pip install -e .
which pdf2md
```

## install from github

```bash
# on ubuntu
sudo apt install python3.12-venv
python3 -m venv py_env
source py_env/bin/activate
which python3
# /home/michael/py_env/bin/python3
pip3 list
# Package Version
# ------- -------
# pip     24.0

python3 -m pip install "git+https://github.com/michaelwfc/computer_network.git#subdirectory=pdf2md"
```