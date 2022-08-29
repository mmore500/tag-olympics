from ._lookup_metric_priority import lookup_metric_priority

def lookup_metric_color(metric_name: str) -> str:
    colors = {
        'Hash' : 'C0',
        'Hamming' : 'C1',
        'Integer' : 'C2',
        'Integer (bi)' : 'C3',
        'Streak' : 'C4',
    }
    return colors[metric_name]
