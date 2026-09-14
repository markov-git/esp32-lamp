function toggle() {
    const nextValue = document.getElementById('sw').classList.contains('on')
        ? '0'
        : '1'
    const url = '/t?s=' + nextValue;
    fetch(url)
        .then((r) => r.text())
        .then((d) => {
            if (d == '1') {
                document.getElementById('sw').classList.add('on');
                document.getElementById('st').textContent = 'ON';
            } else {
                document.getElementById('sw').classList.remove('on');
                document.getElementById('st').textContent = 'OFF';
            }
        })
}