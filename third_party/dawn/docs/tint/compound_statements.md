# Compound Statements

Compound statements are statements that can hold other statements.

This document maps the WGSL compound statements to their semantic tree representations.

## if statement

WGSL:
```
if (condition_a) {
    statement_a;
} else if (condition_b) {
    statement_b;
} else {
    statement_c;
}
```

Semantic tree:
```
sem::IfStatement {
    condition_a
    sem::BlockStatement {
        statement_a
    }
    sem::IfStatement {
        condition_b
        sem::BlockStatement {
            statement_b
        }
        sem::BlockStatement {
            statement_c
        }
    }
}
```

## for loop

WGSL:
```
for (initializer; condition; continuing) {
    statement;
}
```

Semantic tree:
```
sem::ForLoopStatement {
    sem::Statement  initializer
    sem::Expression condition
    sem::Statement  continuing

    sem::LoopBlockStatement {
        sem::Statement statement
    }
}
```

## while

WGSL:
```
while (condition) {
    statement;
}
```

Semantic tree:
```
sem::WhileStatement {
    sem::Expression condition

    sem::LoopBlockStatement {
        sem::Statement statement
    }
}
```

## loop

WGSL:
```
loop (condition) {
    statement_a;
    continuing {
        statement_b;
    }
}
```

Semantic tree:
```
sem::LoopStatement {
    sem::Expression condition

    sem::LoopBlockStatement {
        sem::Statement statement_a
        sem::LoopContinuingBlockStatement {
            sem::Statement statement_b
        }
    }
}
```


## switch statement

WGSL:
```
switch (condition) {
    case literal_a, literal_b: {
        statement_a;
    }
    default {
        statement_b;
    }
}
```

Semantic tree:
```
sem::SwitchStatement {
    sem::Expression condition
    sem::CaseStatement {
        sem::BlockStatement {
            sem::Statement statement_a
        }
    }
    sem::CaseStatement {
        sem::BlockStatement {
            sem::Statement statement_b
        }
    }
}
```
