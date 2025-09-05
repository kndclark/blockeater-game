# Makefile for Python game development environment
# Uses a virtual environment and installs dependencies from requirements.txt
PYTHON= /usr/bin/python3
VENV_NAME= pydev
VENV_DIR= $(CURDIR)/$(VENV_NAME)

venv: 
	$(PYTHON) -m venv $(VENV_NAME)
	$(VENV_NAME)/bin/pip install -r requirements.txt
	echo "To activate the virtual environment, run: source $(VENV_NAME)/bin/activate"

# run:
# 	$(PYTHON) src/game_test.py

clean:
	rm -f __pycache__/*

.PHONY: run clean