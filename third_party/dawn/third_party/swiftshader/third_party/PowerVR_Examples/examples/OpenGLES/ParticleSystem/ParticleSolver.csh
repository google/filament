#version 310 es


//Please #define WORKGROUP_SIZE from the code side

//Binding points
#define SPHERES_UBO_BINDING 1
#define CONFIG_UNIFORM_BINDING 2
#define PARTICLES_SSBO_BINDING_IN 3
#define PARTICLES_SSBO_BINDING_OUT 4

//Arbitrary factors to make physics work in a visually pleasing way
#define TIME_FACTOR .1
#define VELOCITY_FACTOR 4.

#define numSpheres 8u

// Struct declarations to have a nice representation of the data layout.
// Please be aware of the alignment restrictions (e.g. sizeof(vec3) == sizeof(vec4)) that are defined in the GLSL specification.
// Because of that we need to be very careful, and upload the data in a controllable way from the CPU (e.g. using std140 packing,
// packing manually float values in quads and reading as vectors et.c.).

struct Particle
{
	highp vec3 vPosition;
	mediump float _padding_particle_0;
	mediump vec3 vVelocity;
	mediump float fLife;
};

struct Emitter
{
	highp mat4 mTransformation;
	mediump float fHeight;
	mediump float fRadius;
};

layout(std140, binding = PARTICLES_SSBO_BINDING_IN) readonly buffer SsboParticlesInput
{
	Particle aParticlesIn[];
};
layout(std140, binding = PARTICLES_SSBO_BINDING_OUT) writeonly buffer SsboParticlesOut
{
	Particle aParticlesOut[];
};

layout(std140, binding = SPHERES_UBO_BINDING) uniform UboSpheres
{
	highp vec4 aSpheres[8];
};

layout(std140, binding = CONFIG_UNIFORM_BINDING) uniform UboData
{
	Emitter emitter;
	highp vec3 gravity;
	highp float dt;
	highp float fTotalTime;
	highp float dragCoeffLinear;
	highp float dragCoeffQuadratic;
	highp float inwardForceCoeff;
	highp float inwardForceRadius;
	highp float bounciness;
	highp float minLifespan;
	highp float maxLifespan;
};

int last_random;
void seed_random(int seed)
{
	last_random = seed;
}

float randf()
{
	// This is not random at all, and was chosen for looking nice and nothing else.
	last_random = abs(1103515245 * abs(last_random) + 12345);

	return float(last_random) / float(2147483647);
}

// Emits the aParticles[gid] at a random position within the emitter params
Particle EmitParticle(Emitter emitter)
{
	// Generate random parameters for spawn position calculations
	highp float u = randf() * emitter.fHeight;
	highp float r = randf() * emitter.fRadius;

	//We need pos to be a vec4 so we can transform it using the emitter's transformation
	mediump vec4 emittedDirection = vec4(r, u, r, 1);

	// Transform according to emitter orientation
	emittedDirection = emitter.mTransformation * emittedDirection;

	Particle particle;
	// Give it random speed
	particle.vVelocity = emittedDirection.xyz * vec3(7., 15., 7.) + vec3(0., 15., 0.);

	//Start from the origin (this could have been randomised as well.
	particle.vPosition = vec3(0, 0, 0);

	return particle;
}

// Main kernel entry point. Simulates the aParticles[gid] movement and collides it against collider geometry (fixed ground plane at y=0).
layout(local_size_x = WORKGROUP_SIZE) in;
void main()
{
	uint gid = gl_GlobalInvocationID.x;

	seed_random(int(fTotalTime * 13579846.) + int(gid + 10000u) * 145);

	/*We remove the following bounds check because our code has made sure that the particles are a multiple of our workgroup*/
	//const uint numParticles = 131072; //or numParticles passed as a uniform, or as a compile-time constant
	//if (gid >= numParticles) return; 

	// Load a particle's values in a register.
	Particle particle = aParticlesIn[gid];

	mediump float life = particle.fLife;

	// Reduce it's lifetime ( 0 < life < in the order of 1)
	life -= dt * TIME_FACTOR;

	// Respawn if particle is dead 
	if (life < .0)
	{
		particle = EmitParticle(emitter);
		life = randf() * (maxLifespan - minLifespan) + minLifespan;
	}
	else
	{
		mediump vec3 v = particle.vVelocity;

		//Simulate particle movement: 
		// Mass is implicitly "1" just to simplify calculations.
		//Semi-implicit Euler is the best of the "completely basic", forward integration methods - it has better
		//behaviour than explicit Euler basically for free. Error is of the same order as Explicit Euler, but it 
		//is much more stable as it normally retains energy.
		//Explicit Euler is		    : x1 = x0 + v0.dt, then: v1 = v0 + a0*dt

		//Semi - Implicit Euler is 	: v1 = v0 + a0*dt, then: x1 = x0 + v1*dt (we will use this)
		//OR					    : x1 = x0 + v0*dt, then: v1 = v0 + a1*dt


		highp vec3 Ftotal = gravity;

		//CENTERWARD ATTRACTION (Make a pillar)
		if (inwardForceCoeff != 0.)
		{
			mediump float inward = (particle.vPosition.x * particle.vPosition.x + particle.vPosition.z * particle.vPosition.z) * inwardForceCoeff;
			if (inward > .001)
			{
				Ftotal += max(inward - inwardForceRadius + life, 0.) * normalize(vec3(-particle.vPosition.x, 0, -particle.vPosition.z));
			}
		}

		//DRAG (Linear (shape) and Quadratic (medium)
		if (dragCoeffLinear != 0. && dragCoeffQuadratic != 0.)
		{
			mediump float vlength = length(v);
			highp vec3 vnorm = v / vlength;
			Ftotal += vnorm * (vlength * (-dragCoeffLinear - dragCoeffQuadratic * vlength));
		}

		//a0 = Ftot. Update velocity
		v = Ftotal * dt + v;

		//Do the last part of the semi-implicit Euler : 
		particle.vPosition = v * dt + particle.vPosition;

		//Then, collide against the spheres (very fake, but looks convincing)
		for (uint i = 0u; i < numSpheres; i++)
		{
			// Fetch sphere attributes (position=sphereDef.xyz, radius=sphereDef.w)
			highp vec4 sphereDef = aSpheres[i];

			//Check whether particle penetrated the sphere. Reuse calculations as much as possible.
			//Normally, we would just go ahead and use length(), but at this point we will probably
			//be using a lot of the intermediates as well.
			highp vec3 sphere_to_particle = particle.vPosition - sphereDef.xyz;
			highp float distanceSq = dot(sphere_to_particle, sphere_to_particle);

			//If inside the sphere
			if (distanceSq < sphereDef.w * sphereDef.w)
			{
				highp float rDist = inversesqrt(distanceSq);
				//Project it back to sphere surface and do a very simple and fast velocity inversion
				vec3 dir_to_circle = sphere_to_particle * rDist;
				particle.vPosition = dir_to_circle * sphereDef.w + sphereDef.xyz; //Move it ON the sphere
				v = reflect(v, dir_to_circle) * bounciness; //Velocity pointing outwards of the sphere
				break;
			}
		}

		// Collide against ground plane
		if (particle.vPosition.y < 0.0f)
		{
			//	//x & z coeffs represent a sort of friction, y represents reflection -- As always, we can
			//	//use a model that is as physically correct or incorrect as is appropriate for our use
			const highp vec3 reflectCoeffs = vec3(0.4f, -0.3f, 0.4f);
			particle.vPosition.y = -particle.vPosition.y;
			v *= reflectCoeffs;
		}

		//Pack velocity back
		particle.vVelocity = v;
	}
	//Pack life back
	particle.fLife = life;

	//Write to global memory
	aParticlesOut[gid] = particle;

	//We do not synchronize as there is absolutely no interaction between particles in that case
}
