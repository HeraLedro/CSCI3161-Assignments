-------ASTEROIDS.CPP README------

    This is an implementation of the Asteroids game in OpenGL. Despite being
    formatted as a .cpp program, this is also a functional C program and can be
    compiled with both g++ and gcc.


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


---Aspects---




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
        polish aspect. I currently use a

    Remove Redundancies, Bugs
        As with any development project, along the way there were a build-up
        of code redundancies and minor bugs left untreated. I'd like to pursue
        fixing the bugs and removing the redundancies.
