# A Parallelized Objective Function of Delivery Logistic Problem <!-- omit in toc -->

---
## Table of Contents <!-- omit in toc -->
- [Abstract](#abstract)
- [General Aim](#general-aim)
- [Context](#context)
- [Problem](#problem)
- [Methodology](#methodology)
  - [Objective Function](#objective-function)
  - [Technologies and Tools](#technologies-and-tools)
  - [Experimental Design](#experimental-design)
- [Installation and Usage](#installation-and-usage)
- [Experiments](#experiments)
- [Analysis of Results](#analysis-of-results)
- [Author, Affiliation and Contact](#author-affiliation-and-contact)
- [License](#license)
- [References](#references)

---
## Abstract

---
## General Aim
Develop an parallelized objective function based on flatten centroids representation for comparing results in [An algorithm to compute time-balanced clusters for the delivery logistics problem](https://doi.org/10.1016/j.engappai.2022.104795) with [Differential Evolution](https://link.springer.com/article/10.1023/a:1008202821328) and [Genetic Algorithm](https://books.google.com.mx/books?id=5EgGaBkwvWcC&lpg=PR7&ots=mKmq2YMnxo&dq=Adaptation%20in%20Natural%20and%20Artificial%20Systems&lr&pg=PR5#v=onepage&q&f=false) metaheuristics.

---
## Context
The distribution of goods across a geographic region constitutes the primary challenge for many logistics companies, as they consistently seek to reduce costs and delivery times while complying with labor regulations concerning employee workloads. Consequently, the development of optimal delivery routes has become a central issue of significant real-world relevance, aligning with the growth of commercial enterprises that increasingly require greater delivery efficiency (Menchaca-Méndez et al., 2022, p. 1).

---
## Problem
"A company expects to organize deliveries to a set $Q$ of points across a city and has the constraint of assigning an equivalent number of working hours to each worker. Thus, the sale points must be organized in disjoint routes that, considering travel and delivery times, allow the drivers to work approximately the same number of hours and not exceed the time $t_{max}$ specified in their contract. Moreover, each route must correspond to a well-defined city area, with no intersection between the different zones." (Menchaca-Méndez et al., 2022, p. 1).

---
## Methodology
We propose to find the optimal centroids which form well-defined, time-balanced clusters using population-based metaheuristics for continuos spaces. For this project, we are only developing the objective function for the optimization problem and the results comparison between metaheuristics will be done in another project.

### Objective Function
For this optimization problem, we propose an objective function that assigns every point in $Q$ to its closer centroid $C$ based on haversine metric and return the standard deviation of the deliver times of the formed clusters.  

Each solution is a list of centroids of size $(k,D)$ where $k$ is the number of clusters to form and $D$ are the dimensions or features of each point, which to be flattened generates a solution of size $kD$. This representation is the expected input of the objective function, which algorithm is the next:

```mermaid
```

### Technologies and Tools

### Experimental Design

---
## Installation and Usage

---
## Experiments

---
## Analysis of Results

---
## Author, Affiliation and Contact
Alexis Aguilar [Student of Bachelor's Degree in "Tecnologías para la Información en Ciencias" at Universidad Nacional Autónoma de México [UNAM](https://www.unam.mx/)]: alexis.uaguilaru@gmail.com

Project developed for the subjects "High-Performance Computing (HPC)" and "Evolutionary Computation" taught in semestre 2026-2.

---
## License

---
## References
1. Holland, J. H. (1992). Adaptation in natural and artificial systems: an introductory analysis with applications to biology, control, and artificial intelligence. MIT press.
2. Menchaca-Méndez, A., Montero, E., Flores-Garrido, M., & Miguel-Antonio, L. (2022). An algorithm to compute time-balanced clusters for the delivery logistics problem. Engineering Applications of Artificial Intelligence, 111, 104795. https://doi.org/https://doi.org/10.1016/j.engappai.2022.104795
3. Storn, R., & Price, K. (1997). Differential evolution–a simple and efficient heuristic for global optimization over continuous spaces. Journal of global optimization, 11 (4), 341-359.