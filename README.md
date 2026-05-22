#  Robotic Hand for Carina

University team project focused on developing a functional robotic hand prototype for assistive use in a real-world scenario.

---

## Overview

This project was developed as part of a multidisciplinary university team with the objective of designing a low-cost assistive robotic hand adapted to a real user.

Rather than approaching the project as a purely theoretical robotics exercise, much of the development process revolved around practical usability, physical constraints, reliability, and iterative problem solving around a real person’s needs.

The project ultimately achieved **4th place in a national university engineering competition**.

---

## The Real Challenge


Very early in development, we realized that building a robotic hand was not the difficult part. Building one that could actually be used reliably by a real person was.

The project required balancing mechanical simplicity, limited manufacturing resources, low-cost electronics, usability, and competition constraints. Many ideas that initially looked promising failed completely during real-world testing.

---

## Designing Around a Real User

One of the first major challenges was anchoring the prosthetic securely to Carina’s residual limb.

Our initial approach involved creating a physical mold of her arm using a body-casting material capable of capturing detailed geometry. The resulting mold was then 3D scanned using university equipment from UdG, allowing us to design a custom-fit attachment for the prosthetic.

On paper, the solution seemed ideal:
- personalized fit,
- accurate geometry,
- improved comfort.

In practice, it performed poorly during real usage.

The socket lacked sufficient stability under movement and load, especially during competition tasks involving lifting and repetitive motion. What initially appeared to be a precise engineering solution became unreliable once subjected to dynamic use.

After several iterations, we progressively added reinforcements, anchoring bands, extended support surfaces, and additional fixation points. The final system was less elegant than the original concept, but considerably more functional and reliable.

This became a recurring lesson throughout the project: practical robustness consistently mattered more than theoretical elegance.

![Early CAD concepts](cad/addesive_OGversion)

---

## Control System Challenges

The original goal was to support three independent control signals for multiple hand movements. In practice, this proved difficult due to the limited available muscle area and the low precision of the EMG sensors we could afford.

Since the residual limb ended before the elbow, reliably distinguishing different muscle activations became inconsistent during repeated use.

Rather than continuing toward a more complex but unreliable system, we simplified the interaction model. The final approach relied on contraction timing thresholds, where single and consecutive contractions triggered different grip modes.

Although simpler than originally intended, the system proved substantially more usable and reliable during testing. This tradeoff between complexity and reliability became one of the most important engineering decisions of the project.

---

## Mechanical Design Iterations

The hand design evolved continuously throughout development. Different team members proposed different finger counts, movement systems, and mechanical architectures before the group progressively converged on a design that balanced manufacturability, available time, reliability, and competition requirements.

The final prototype used a three-finger configuration with piston-driven finger contraction forming a pincer-style grip. While initially perceived as a simplified solution, the design turned out to be surprisingly effective for most competition tasks.

The project reinforced the importance of designing for repeatability and robustness rather than maximum mechanical complexity.

---

## Failures During Testing

A significant portion of development involved solving failures that only appeared during repeated real-world testing.

Some of the most problematic issues included structural fractures during lifting tasks, unstable batteries damaging electronic components, unreliable sensor readings, and attachment failures under movement.

Many solutions near the end of development became highly pragmatic due to time and reliability constraints before the competition. In retrospect, these failures became one of the most valuable parts of the experience, forcing the team to continuously adapt the design under realistic operating conditions.

---

## Competition

The final prototype was presented in a national university engineering competition focused on assistive technology and robotics.

The competition involved practical tasks requiring object manipulation, grip reliability, movement consistency, and resistance under repeated operation.

Beyond the final ranking, the experience provided direct exposure to multidisciplinary collaboration, iterative hardware development, and user-centered engineering under real constraints.

![In medias res](competition_26_record/IMG_4589)

---

## Reflections

One of the most important lessons from this project was understanding how different engineering becomes once a real user is involved.

Many solutions that appeared technically correct during development failed when confronted with comfort, repeatability, fatigue, usability, and physical constraints.

The project became less about building a robotic hand, and more about continuously adapting engineering decisions to reality.

---

## Repository Disclaimer

This repository is intended as a documentation and showcase of the project experience.

The robotic hand, electronics, software, and mechanical systems were developed collaboratively as part of a university team effort. For this reason, the repository focuses primarily on the engineering process, design decisions, and project evolution rather than providing a complete implementation archive.

Check our social media:
https://magironica.udg.edu/#team
https://www.linkedin.com/feed/update/urn:li:activity:7455873868923154432/

Competition website:
https://sites.google.com/view/competicion-manos-upv/inicio?authuser=0
