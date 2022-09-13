from ._lookup_metric_priority import lookup_metric_priority

def lookup_metric_color(metric_name: str) -> str:
    colors = {
        'Hash' : 'C0',
        'Hamming' : 'C1',
        'Streak' : 'C2',
        'Integer' : 'C3',
        'Integer (bi)' : 'C4',
    }
    return colors[metric_name]
