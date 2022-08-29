
def lookup_metric_priority(metric_name: str) -> int:

    priority_order = [
        'Hash', # put hash first because it is control
        'Hamming',
        'Integer',
        'Integer (bi)',
        'Streak',
    ]

    return priority_order.index(metric_name)
