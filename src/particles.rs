use macroquad::prelude::*;

#[derive(Debug, Clone, Copy)]
pub struct Particle {
    pub pos: Vec2,
    pub vel: Vec2,
    pub color: Color,
    pub life: f32,
    pub max_life: f32,
    pub size: f32,
    pub active: bool,
}

impl Default for Particle {
    fn default() -> Self {
        Self {
            pos: Vec2::ZERO,
            vel: Vec2::ZERO,
            color: WHITE,
            life: 0.0,
            max_life: 1.0,
            size: 4.0,
            active: false,
        }
    }
}

pub struct ParticleSystem {
    particles: Vec<Particle>,
    next_index: usize,
}

impl Default for ParticleSystem {
    fn default() -> Self {
        Self::new()
    }
}

impl ParticleSystem {
    pub const POOL_SIZE: usize = 400;

    pub fn new() -> Self {
        let mut particles = Vec::with_capacity(Self::POOL_SIZE);
        for _ in 0..Self::POOL_SIZE {
            particles.push(Particle::default());
        }
        Self {
            particles,
            next_index: 0,
        }
    }

    pub fn spawn(&mut self, pos: Vec2, vel: Vec2, color: Color, size: f32, lifetime: f32) {
        let p = &mut self.particles[self.next_index];
        p.pos = pos;
        p.vel = vel;
        p.color = color;
        p.size = size;
        p.life = lifetime;
        p.max_life = lifetime;
        p.active = true;

        self.next_index = (self.next_index + 1) % Self::POOL_SIZE;
    }

    pub fn burst(&mut self, pos: Vec2, color: Color, count: usize, speed: f32, size: f32) {
        use ::rand::Rng;
        let mut rng = ::rand::thread_rng();

        for _ in 0..count {
            let angle = rng.gen_range(0.0..std::f32::consts::TAU);
            let spd = rng.gen_range(speed * 0.3..speed);
            let vel = Vec2::new(angle.cos() * spd, angle.sin() * spd);
            let life = rng.gen_range(0.25..0.65);
            self.spawn(pos, vel, color, size, life);
        }
    }

    pub fn update(&mut self, dt: f32) {
        for p in self.particles.iter_mut() {
            if !p.active {
                continue;
            }
            p.life -= dt;
            if p.life <= 0.0 {
                p.active = false;
            } else {
                p.pos += p.vel * dt;
            }
        }
    }

    pub fn draw(&self, scale: f32, offset: Vec2) {
        for p in self.particles.iter() {
            if !p.active {
                continue;
            }
            let alpha = (p.life / p.max_life).clamp(0.0, 1.0);
            let mut col = p.color;
            col.a *= alpha;

            let render_pos = p.pos * scale + offset;
            let render_size = (p.size * scale * (0.4 + 0.6 * alpha)).max(1.0);

            draw_rectangle(
                render_pos.x - render_size * 0.5,
                render_pos.y - render_size * 0.5,
                render_size,
                render_size,
                col,
            );
        }
    }

    pub fn clear(&mut self) {
        for p in self.particles.iter_mut() {
            p.active = false;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_particle_pool_lifecycle() {
        let mut ps = ParticleSystem::new();
        assert_eq!(ps.particles.len(), ParticleSystem::POOL_SIZE);

        ps.spawn(Vec2::new(10.0, 10.0), Vec2::new(5.0, 0.0), WHITE, 4.0, 1.0);
        assert!(ps.particles[0].active);
        assert_eq!(ps.particles[0].pos, Vec2::new(10.0, 10.0));

        // Advance time by 0.5s
        ps.update(0.5);
        assert!(ps.particles[0].active);
        assert_eq!(ps.particles[0].pos.x, 12.5);

        // Advance time past lifetime (1.0s)
        ps.update(0.6);
        assert!(!ps.particles[0].active);

        // Clear all
        ps.spawn(Vec2::ZERO, Vec2::ZERO, WHITE, 4.0, 1.0);
        ps.clear();
        assert!(!ps.particles[1].active);
    }
}
