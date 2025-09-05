# Simple 2D game window using pygame
import pygame
import sys

pygame.init()
screen = pygame.display.set_mode((640, 480))
pygame.display.set_caption("Simple 2D Game")

clock = pygame.time.Clock()

while True:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            sys.exit()

    screen.fill((30, 30, 30))  # Fill screen with dark gray
    pygame.display.flip()
    clock.tick(60)  # Limit to 60 FPS