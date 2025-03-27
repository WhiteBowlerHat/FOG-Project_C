# Projet Fog

Here is a simple flow chart:

```mermaid
flowchart LR;
 subgraph s1["GNPA (SC)"];
    direction LR;
        n3["Constante"];
        n1["Cléf secrète"];
        n2["Nonce"];
        
  end;
    A["0,0,0,0,..."] --> XOR@{ shape: cross-circ, label: "Summary" };
    s1 --> XOR;
    XOR --> ["2,8,9,4,..."]
```