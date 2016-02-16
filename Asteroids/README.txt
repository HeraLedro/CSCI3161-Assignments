-------ASTEROIDS.CPP README------

    This is an implementation of the Asteroids game in OpenGL. Despite being
    formatted as a .cpp program, this is also a functional C program and can be
    compiled with both g++ and gcc.


---Controls---

    Movement
        Turn Left       =   [LEFT ARROW]
        Turn Right      =   [RIGHT ARROW]
        Move Forward    =   [FORWARD ARROW]
        Move Backward   =   [BACK ARROW]

    Photon
        Fire = [SPACE]

    Change Asteroid Shape
        Change Between Circle and Polygon   =   [S]

    Quit Game   =   [Q]


---Process---

    The primary problem in this program was the ship movement - understanding
    how the rotate and translate functions worked on the ship and how the ship's
    coordinates were determined was a hurdle. This took about a week to fully
    comprehend and implement. It works as follows:

        1)  Use keys to modify ship velocity <dx,dy> and angle (phi).
        2)  Move ship coordinates each frame based on velocity <dx,dy>
        3)  Draw the ship based on the relative coordinates of its vertices.

    Similar processes are used for the photon and asteroids, with the exceptions
    being that photons do not require rotation and asteroids require a rotation
    by an angle (dphi) at each frame.


---Game Objects---

    Ship
        The main player sprite. With the press of a button, the ship can turn
        left or right and fly forward or back based on its current orientation.
        A player can fire a photon from the ship. When the ship center leaves
        the boundaries of the screen, the ship is moved to the other side of the
        appropriate axis.

    Asteroid
        The object of the game is to destroy these asteroids. They fly around
        the screen with random initial trajectories and rotations. If a large
        enough asteroid is destroyed, it will spawn two smaller asteroids which
        split apart and travel based on the trajectory of their parent. An
        asteroid can be destroyed by colliding with a photon. Collision between
        a ship and an asteroid ends in a game over and restarts the game. Like
        ships, asteroids 'wrap' around the screen.

    Photon
        The sprite fired by the player. Any photon which impacts an asteroid
        destroys it. Photons do not wrap around the screen like asteroids.


---Left to Implement---

    There were several features that I wanted to implement, but ran out of time
    necessary to do so. These are listed in the TODO.txt file and detailed here.

    Code Modulation
        This is more of a personal irritation - I dislike having all of my code
        within a single file. For the purposes of this assignment it is
        acceptable, but in the future I would like to split this code into its
        individual parts.

    Animation Effects
        Making asteroids and the ship explode are polish effects. As it is, the
        game 'pauses' on death to see what happened at the time of death. In the
        future I would like to explore ways of making the ship and asteroids
        explode.

    Score Display
        This is an elementary polish feature that would not take long to
        implement, but still requires more time than I was able to allot. Simply
        put, this would increment a score value for each asteroid destroyed. The
        score for each asteroid would increase inversely to its size - because
        of the difficulty faced with hitting smaller asteroids, they would be
        worth more in comparison to their larger parents.

    Strict Polygon-to-Polygon Collision
        For the purposes of this assignment, polygon-to-polygon collision a
        polish aspect. I currently use a point-to-circle collision detection
        to check if my ships or photons are reasonably within the bounds of my
        asteroid. In the future I'd like to implement a legitimate polygon-to-
        polygon collision detection.

    Remove Redundancies, Bugs
        As with any development project, along the way there were a build-up
        of code redundancies and minor bugs left untreated. I'd like to pursue
        fixing the bugs and removing the redundancies.
