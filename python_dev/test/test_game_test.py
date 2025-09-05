import pygame
import sys
import pytest

def test_window_initialization(monkeypatch):
    # Patch sys.exit to prevent exiting the test runner
    monkeypatch.setattr(sys, "exit", lambda: None)
    pygame.init()
    screen = pygame.display.set_mode((640, 480))
    pygame.display.set_caption("Simple 2D Game")
    assert screen.get_width() == 640
    assert screen.get_height() == 480
    pygame.quit()

def test_quit_event(monkeypatch):
    # Patch sys.exit to prevent exiting the test runner
    monkeypatch.setattr(sys, "exit", lambda: None)
    pygame.init()
    screen = pygame.display.set_mode((640, 480))
    # Post a QUIT event to the event queue
    pygame.event.post(pygame.event.Event(pygame.QUIT))
    quit_event_found = False
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            quit_event_found = True
    assert quit_event_found