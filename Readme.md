# Projet Fog

Here is a simple flow chart:

```mermaid
flowchart LR;
 subgraph s1["GNPA (SC)"];
    direction LR;
        n3["Constante"];
        n1["Cléf secrète"];
        n2["Nonce"];
  end
    A["0,0,0,0,..."] --0,0,0...--> n4;
    s1 --f1,f2,f3,...--> n4;
    n4 --> B["0,2,9,1,..."];
    n4["Summary"];
    n4@{ shape: summary};

```